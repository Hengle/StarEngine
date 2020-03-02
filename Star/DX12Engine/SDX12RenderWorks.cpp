// Copyright (C) 2019-2020 star.engine at outlook dot com
//
// This file is part of StarEngine
//
// StarEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// StarEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with StarEngine.  If not, see <https://www.gnu.org/licenses/>.

#include "SDX12RenderWorks.h"
#include "SDX12Types.h"
#include "SDX12ShaderDescriptorHeap.h"
#include <Star/Graphics/SRenderUtils.h>

namespace Star::Graphics::Render {

void createRenderSolutionRenderTargets(ID3D12Device* pDevice, IDXGISwapChain3* pSwapChain,
    DX12ShaderDescriptorHeap* pDescriptorHeap, DX12RenderWorks& rw, uint32_t id, uint32_t pipelineID
) {
    //-------------------------------------------------------------------
    // SwapChain
    // Create render targets 
    for (uint32_t i = 0; i != rw.mSolutions[id].mFramebuffers.size(); ++i) {
        const auto& rt = rw.mSolutions[id].mFramebuffers[i];

        if (i < rw.mNumBackBuffers) {
            Expects(!rw.mFramebuffers[i]);
            V(pSwapChain->GetBuffer(i, IID_PPV_ARGS(rw.mFramebuffers[i].put())));
            //STAR_SET_DEBUG_NAME(mFramebuffers[i], sc.mName + std::to_string(i));
        } else {
            using namespace Graphics::Render;

            D3D12_RESOURCE_DESC desc = {};
            desc.Dimension = getDX12(rt.mResource.mDimension);

            desc.Alignment = rt.mResource.mAlignment;
            desc.Width = rt.mResource.mWidth;
            desc.Height = rt.mResource.mHeight;
            desc.DepthOrArraySize = rt.mResource.mDepthOrArraySize;
            desc.MipLevels = rt.mResource.mMipLevels;
            desc.Format = getDXGIFormat(rt.mResource.mFormat);
            desc.SampleDesc.Count = rt.mResource.mSampleDesc.mCount;
            desc.SampleDesc.Quality = rt.mResource.mSampleDesc.mQuality;
            desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
            desc.Flags = static_cast<D3D12_RESOURCE_FLAGS>(rt.mResource.mFlags);
            
            D3D12_HEAP_PROPERTIES heap{
                D3D12_HEAP_TYPE_DEFAULT, D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
                D3D12_MEMORY_POOL_UNKNOWN, 1u, 1u
            };
        
            if (desc.Format == DXGI_FORMAT_UNKNOWN) { // skip empty framebuffer
                throw std::runtime_error("empty render target");
            }

            visit(overload(
                [&](const ClearColor& cv) {
                    D3D12_CLEAR_VALUE clearValue = {
                        getDXGIFormat(cv.mClearFormat),
                        { cv.mClearColor.x(), cv.mClearColor.y(), cv.mClearColor.z(), cv.mClearColor.w() }
                    };
                    V(pDevice->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE,
                        &desc, D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue,
                        IID_PPV_ARGS(rw.mFramebuffers[i].put())));
                },
                [&](const ClearDepthStencil& cv) {
                    D3D12_CLEAR_VALUE clearValue = {
                        getDXGIFormat(cv.mClearFormat),
                    };
                    clearValue.DepthStencil = { cv.mDepthClearValue, cv.mStencilClearValue };
                    V(pDevice->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE,
                        &desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue,
                        IID_PPV_ARGS(rw.mFramebuffers[i].put())));
                }
            ), rt.mClear);
        }
    }

    auto& sl = rw.mSolutions[id];
    for (uint32_t i = 0; i != sl.mRTVs.size(); ++i) {
        const auto& rt = rw.mFramebuffers[sl.mRTVSources[i].mHandle];
        const auto& desc0 = sl.mRTVs[i];
        D3D12_RENDER_TARGET_VIEW_DESC desc;
        desc.Format = getDXGIFormat(desc0.mFormat);
        desc.ViewDimension = (D3D12_RTV_DIMENSION)desc0.mViewDimension;
        desc.Texture2D = getDX12(desc0.mTexture2D);

        pDevice->CreateRenderTargetView(rt.get(), &desc, rw.mRTVs.getCpuHandle(i));
    }
    for (uint32_t i = 0; i != sl.mDSVs.size(); ++i) {
        const auto& ds = rw.mFramebuffers[sl.mDSVSources[i].mHandle];
        const auto& desc0 = sl.mDSVs[i];

        D3D12_DEPTH_STENCIL_VIEW_DESC desc;
        desc.Format = getDXGIFormat(desc0.mFormat);
        desc.ViewDimension = (D3D12_DSV_DIMENSION)desc0.mViewDimension;
        desc.Flags = (D3D12_DSV_FLAGS)desc0.mFlags;
        desc.Texture2D = getDX12(desc0.mTexture2D);

        pDevice->CreateDepthStencilView(ds.get(), &desc, rw.mDSVs.getCpuHandle(i));
    }

    uint32_t cbv_srv_uavIndex = 0;

    for (uint32_t i = 0; i != sl.mSRVs.size(); ++i) {
        const auto& rt = rw.mFramebuffers[sl.mSRVSources[i].mHandle];
        const auto& desc0 = sl.mSRVs[i];
        D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
        desc.Format = getDXGIFormat(desc0.mFormat);
        desc.ViewDimension = getDX12(desc0.mViewDimension);
        desc.Shader4ComponentMapping = desc0.mShader4ComponentMapping;
        desc.Texture2D = getDX12(desc0.mTexture2D);
        pDevice->CreateShaderResourceView(rt.get(), &desc, rw.mCBV_SRV_UAVs.getCpuHandle(cbv_srv_uavIndex));
        ++cbv_srv_uavIndex;
    }

    auto& pipeline = sl.mPipelines[pipelineID];
    for (auto& pass : pipeline.mPasses) {
        for (auto& subpass : pass.mGraphicsSubpasses) {
            for (auto& collection : subpass.mDescriptors) {
                for (auto& dx12List : collection.mResourceViewLists) {
                    visit(overload(
                        [&](Persistent_) {
                            if (collection.mIndex.mUpdate < PerPass)
                                return;

                            auto descs = pDescriptorHeap->allocatePersistent(dx12List.mCapacity);
                            dx12List.mGpuOffset = descs.first.mGpuHandle;
                            dx12List.mCpuOffset = descs.first.mCpuHandle;
                            uint32_t offset = 0;
                            for (const auto& range : dx12List.mRanges) {
                                for (const auto& subrange : range.mSubranges) {
                                    size_t i = 0;
                                    visit(overload(
                                        [&](EngineSource_) {
                                            for (const auto& attr : subrange.mDescriptors) {
                                                if (std::holds_alternative<SamplerState_>(attr.mAttributeType)) {
                                                    throw std::runtime_error("resource view should not have samplers");
                                                } else if (!isConstant(attr.mAttributeType)) {
                                                    throw std::runtime_error("GraphicsSubpass EngineSource Descriptor not supported yet");
                                                } else {
                                                    throw std::runtime_error("descriptor should not be constant");
                                                }
                                            }
                                        },
                                        [&](RenderTargetSource_) {
                                            for (const auto& attr : subrange.mDescriptors) {
                                                auto id = sl.mAttributeIndex.at(attr.mID);
                                                auto ptr = rw.mCBV_SRV_UAVs.getCpuHandle(id);
                                                pDevice->CopyDescriptorsSimple(1, descs[i].mCpuHandle, ptr, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
                                                ++i;
                                            }
                                        },
                                        [&](MaterialSource_) {
                                            throw std::runtime_error("GraphicsSubpass MaterialSource Descriptor not supported yet");
                                        }
                                    ), subrange.mSource);
                                    Ensures(i == subrange.mDescriptors.size());
                                    offset += gsl::narrow_cast<uint32_t>(subrange.mDescriptors.size());
                                }
                            }

                            Ensures(offset == dx12List.mCapacity);

                            for (const auto& unbounded : dx12List.mUnboundedDescriptors) {
                                throw std::runtime_error("material resource view list unbounded not supported yet");
                            }
                        },
                        [&](Dynamic_) {
                            // do nothing
                        }
                    ), collection.mIndex.mPersistency);
                }

                for (const auto& dx12List : collection.mSamplerLists) {
                    visit(overload(
                        [&](Persistent_) {
                            for (const auto& range : dx12List.mRanges) {
                                if (std::holds_alternative<SSV_>(range.mType)) {
                                    continue;
                                }
                                throw std::runtime_error("GraphicsSubpass sampler list not supported yet");
                            }
                            for (const auto& unbounded : dx12List.mUnboundedDescriptors) {
                                throw std::runtime_error("material resource view list unbounded not supported yet");
                            }
                        },
                        [&](Dynamic_) {
                        }
                    ), collection.mIndex.mPersistency);
                }
            }
        }
    }
}

void clearRenderTargets(DX12RenderWorks& rw, DX12ShaderDescriptorHeap* pDescriptorHeap, uint32_t solutionID, uint32_t pipelineID) {
    auto& solution = rw.mSolutions[solutionID];
    auto& pipeline = solution.mPipelines[pipelineID];
    for (auto& pass : pipeline.mPasses) {
        for (auto& subpass : pass.mGraphicsSubpasses) {
            for (auto& collection : subpass.mDescriptors) {
                for (auto& dx12List : collection.mResourceViewLists) {
                    visit(overload(
                        [&](Persistent_) {
                            if (dx12List.mCpuOffset.ptr) {
                                pDescriptorHeap->deallocatePersistent(dx12List.mCpuOffset, dx12List.mCapacity);
                                dx12List.mGpuOffset = {};
                                dx12List.mCpuOffset = {};
                            }
                        },
                        [&](Dynamic_) {
                            // do nothing
                        }
                    ), collection.mIndex.mPersistency);
                }
                for (auto& dx12List : collection.mResourceViewLists) {
                    visit(overload(
                        [&](Persistent_) {
                            if (dx12List.mCpuOffset.ptr) {
                                throw std::runtime_error("sampler descriptor not supported yet");
                                //pDescriptorHeap->deallocatePersistent(dx12List.mCpuOffset, dx12List.mCapacity);
                                dx12List.mGpuOffset = {};
                                dx12List.mCpuOffset = {};
                            }
                        },
                        [&](Dynamic_) {
                            // do nothing
                        }
                    ), collection.mIndex.mPersistency);
                }
            }
        }
    }

    rw.mFramebuffers.clear();
}

}
