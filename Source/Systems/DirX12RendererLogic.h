// The rendering system is responsible for drawing all objects
#ifndef DIRX12RENDERERLOGIC_H
#define DIRX12RENDERERLOGIC_H

#include <d3dcompiler.h> //required for compiling shaders on the fly
#pragma comment(lib, "d3dcompiler.lib")
#include "../Utils/d3dx12.h"

// Contains our global game settings
#include "../GameConfig.h"
//Game Data Utilities
#include "../Utils/lvlData.h"

namespace Wing3D
{
	class DirX12RendererLogic
	{
		// shared connection to the main ECS engine
		std::shared_ptr<flecs::world> game;
		// non-ownership handle to configuration settings
		std::weak_ptr<const GameConfig> gameConfig;
		// handle to our running ECS systems
		flecs::system startDraw;
		flecs::system updateDraw;
		flecs::system completeDraw;
		// Used to query screen dimensions
		GW::SYSTEM::GWindow window;
		// DirectX12 resources used for rendering
		GW::GRAPHICS::GDirectX12Surface d3d;
		// what we need at a minimum to draw a triangle
		D3D12_VERTEX_BUFFER_VIEW vertexView;
		D3D12_INDEX_BUFFER_VIEW indexView;
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
		Microsoft::WRL::ComPtr<ID3D12PipelineState>	pipeline;

		// View Matrix for homogeneous position
		GW::MATH::GMATRIXF viewMatrix;
		// Projection Matrix for homogeneous position
		GW::MATH::GMATRIXF projectionMatrix;

		// Struct of Scene Data for GPU
		struct SCENE_DATA {
			// Sun Light settings and Camera Position
			GW::MATH::GVECTORF sunDirection, sunColor, sunAmbiet, camPos;
			// Combined view and projection matrices for homogenization
			GW::MATH::GMATRIXF viewProjection;
		} sceneDataForGPU;

		// Struct of Mesh Data for GPU
		struct MESH_DATA {
			// Indices for color and transform
			unsigned materialIndex, transformIndexStart;
		} meshDataForGPU;

		// Background buffer clear color
		float* backgroundColor;

		// Vector of transforms to update/send to gpu
		std::vector<GW::MATH::GMATRIXF> transformsForGPU;

		// Number of buffers in the swapchain
		UINT maxActiveFrames;

		// All transforms in the level - GPU Resource
		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> transformStrdBuffer;
		// All Materials in the level - GPU Resource
		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>	materialStrdBuffer;

		// Descriptor Heap for Structured Buffers
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;

		// *HARD CODED* sun settings
		GW::MATH::GVECTORF sunLightDir = { -1, -1, 2 }, 
						   sunLightColor = { 0.9f, 0.9f, 1, 1 },
						   sunLightAmbient = { 0.75f, 0.9f, 0.9f, 0 };

		// Delta time tracking
		float														deltaTime;
		std::chrono::steady_clock::time_point						lastUpdate;

		//input handles
		GW::INPUT::GInput ginput;
		GW::INPUT::GController gcontroller;


		// Rendering Log
		GW::SYSTEM::GLog log;

		// Data loaded in from blender
		Level_Data lvlData;

		GW::CORE::GEventReceiver shutdown;
	public:
		// attach the required logic to the ECS 
		bool Init(std::shared_ptr<flecs::world> _game,
			std::weak_ptr<const GameConfig> _gameConfig,
			GW::GRAPHICS::GDirectX12Surface _d3d,
			GW::SYSTEM::GWindow _window, GW::INPUT::GInput ginput, GW::INPUT::GController gcontroller);
		// control if the system is actively running
		bool Activate(bool runSystem);
		// release any resources allocated by the system
		bool Shutdown();

		void SetBackgroundColor(float* bColor);
	private:
		// Setup funcs
		bool SetupShaderVars();
		bool SetupGraphics();


		// Default pipeline
		struct PipelineHandles
		{
			ID3D12GraphicsCommandList* commandList;
			D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView;
			D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView;
		};

		PipelineHandles GetCurrentPipelineHandles()
		{
			PipelineHandles retval;
			d3d.GetCommandList((void**)&retval.commandList);
			d3d.GetCurrentRenderTargetView((void**)&retval.renderTargetView);
			d3d.GetDepthStencilView((void**)&retval.depthStencilView);
			return retval;
		}

		void SetupPipeline(PipelineHandles handles)
		{
			handles.commandList->SetGraphicsRootSignature(rootSignature.Get());
			handles.commandList->SetDescriptorHeaps(1, descriptorHeap.GetAddressOf());
			handles.commandList->OMSetRenderTargets(1, &handles.renderTargetView, FALSE, &handles.depthStencilView);
			handles.commandList->SetPipelineState(pipeline.Get());
			handles.commandList->IASetVertexBuffers(0, 1, &vertexView);
			handles.commandList->IASetIndexBuffer(&indexView);
			handles.commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		}

		void CreatePipelineState(Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, Microsoft::WRL::ComPtr<ID3DBlob> psBlob, ID3D12Device* creator)
		{
			// Create Input Layout
			D3D12_INPUT_ELEMENT_DESC formats[3];
			formats[0].SemanticName = "POSITION";
			formats[0].SemanticIndex = 0;
			formats[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			formats[0].InputSlot = 0;
			formats[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
			formats[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			formats[0].InstanceDataStepRate = 0;

			formats[1].SemanticName = "UVW";
			formats[1].SemanticIndex = 0;
			formats[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			formats[1].InputSlot = 0;
			formats[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
			formats[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			formats[1].InstanceDataStepRate = 0;

			formats[2].SemanticName = "NORMAL";
			formats[2].SemanticIndex = 0;
			formats[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			formats[2].InputSlot = 0;
			formats[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
			formats[2].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			formats[2].InstanceDataStepRate = 0;

			D3D12_GRAPHICS_PIPELINE_STATE_DESC psDesc;
			ZeroMemory(&psDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

			psDesc.InputLayout = { formats, ARRAYSIZE(formats) };
			psDesc.pRootSignature = rootSignature.Get();
			psDesc.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
			psDesc.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());
			psDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			psDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			psDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			psDesc.SampleMask = UINT_MAX;
			psDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			psDesc.NumRenderTargets = 1;
			psDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			psDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
			psDesc.SampleDesc.Count = 1;

			creator->CreateGraphicsPipelineState(&psDesc, IID_PPV_ARGS(&pipeline));
		}

		void CreateRootSignature(ID3D12Device* creator)
		{
			Microsoft::WRL::ComPtr<ID3DBlob> signature, errors;
			CD3DX12_ROOT_PARAMETER rootParams[4] = {};
			CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;

			rootParams[0].InitAsConstants(32, 0);
			rootParams[1].InitAsConstants(2, 1);
			rootParams[2].InitAsShaderResourceView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
			rootParams[3].InitAsShaderResourceView(0, 0, D3D12_SHADER_VISIBILITY_PIXEL);

			rootSignatureDesc.Init(ARRAYSIZE(rootParams), rootParams, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
			D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &errors);

			creator->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
		}

		// Shader Compiling

		void PrintLabeledDebugString(const char* label, const char* toPrint)
		{
			std::cout << label << toPrint << std::endl;
#if defined WIN32 //OutputDebugStringA is a windows-only function 
			OutputDebugStringA(label);
			OutputDebugStringA(toPrint);
#endif
		}

		std::string ReadFileIntoString(const char* filePath)
		{
			std::string output;
			unsigned int stringLength = 0;
			GW::SYSTEM::GFile file;

			file.Create();
			file.GetFileSize(filePath, stringLength);

			if (stringLength > 0 && +file.OpenBinaryRead(filePath))
			{
				output.resize(stringLength);
				file.Read(&output[0], stringLength);
			}
			else
				std::cout << "ERROR: File \"" << filePath << "\" Not Found!" << std::endl;

			return output;
		}

		Microsoft::WRL::ComPtr<ID3DBlob> CompilePixelShader(ID3D12Device* creator, UINT compilerFlags)
		{
			std::string pixelShaderSource = ReadFileIntoString("../Shaders/PixelShader.hlsl");

			Microsoft::WRL::ComPtr<ID3DBlob> psBlob, errors;

			HRESULT compilationResult =
				D3DCompile(pixelShaderSource.c_str(), pixelShaderSource.length(),
					nullptr, nullptr, nullptr, "main", "ps_5_1", compilerFlags, 0,
					psBlob.GetAddressOf(), errors.GetAddressOf());

			if (FAILED(compilationResult))
			{
				PrintLabeledDebugString("Pixel Shader Errors:\n", (char*)errors->GetBufferPointer());
				abort();
				return nullptr;
			}

			return psBlob;
		}

		Microsoft::WRL::ComPtr<ID3DBlob> CompileVertexShader(ID3D12Device* creator, UINT compilerFlags)
		{
			std::string vertexShaderSource = ReadFileIntoString("../Shaders/VertexShader.hlsl");

			Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, errors;

			HRESULT compilationResult =
				D3DCompile(vertexShaderSource.c_str(), vertexShaderSource.length(),
					nullptr, nullptr, nullptr, "main", "vs_5_1", compilerFlags, 0,
					vsBlob.GetAddressOf(), errors.GetAddressOf());

			if (FAILED(compilationResult))
			{
				PrintLabeledDebugString("Vertex Shader Errors:\n", (char*)errors->GetBufferPointer());
				abort();
				return nullptr;
			}
	
			return vsBlob;
		}

		void InitializeGraphicsPipeline(ID3D12Device* creator)
		{
			UINT compilerFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
			compilerFlags |= D3DCOMPILE_DEBUG;
#endif
			Microsoft::WRL::ComPtr<ID3DBlob> vsBlob = CompileVertexShader(creator, compilerFlags);
			Microsoft::WRL::ComPtr<ID3DBlob> psBlob = CompilePixelShader(creator, compilerFlags);
			CreateRootSignature(creator);
			CreatePipelineState(vsBlob, psBlob, creator);
		}

		void InitializeStructuredBuffersAndViews(ID3D12Device* creator)
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorHeap->GetCPUDescriptorHandleForHeapStart());
			for (int i = 0; i < maxActiveFrames; i++)
			{
				unsigned structureBufferSize = sizeof(GW::MATH::GMATRIXF) * lvlData.levelTransforms.size();
				creator->CreateCommittedResource( // using UPLOAD heap for simplicity
					&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // DEFAULT recommend  
					D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(structureBufferSize),
					D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(transformStrdBuffer[i].GetAddressOf()));

				UINT8* transferMemoryLocation;
				transformStrdBuffer[i]->Map(0, &CD3DX12_RANGE(0, 0), reinterpret_cast<void**>(&transferMemoryLocation));
				memcpy(transferMemoryLocation, lvlData.levelTransforms.data(), sizeof(GW::MATH::GMATRIXF) * lvlData.levelTransforms.size());
				transformStrdBuffer[i]->Unmap(0, nullptr);

				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.Buffer.NumElements = lvlData.levelTransforms.size();
				srvDesc.Buffer.StructureByteStride = sizeof(GW::MATH::GMATRIXF);
				srvDesc.Buffer.FirstElement = 0;
				srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

				creator->CreateShaderResourceView(transformStrdBuffer[i].Get(), &srvDesc, handle);
				handle.Offset(1, sizeof(lvlData.levelTransforms));
			}

			for (int i = 0; i < maxActiveFrames; i++)
			{
				unsigned structureBufferSize = sizeof(H2B::ATTRIBUTES) * lvlData.levelMaterials.size();
				creator->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
					D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(structureBufferSize),
					D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(materialStrdBuffer[i].GetAddressOf()));

				UINT8* transferMemoryLocation;
				materialStrdBuffer[i]->Map(0, &CD3DX12_RANGE(0, 0), reinterpret_cast<void**>(&transferMemoryLocation));
				for (int j = 0; j < lvlData.levelMaterials.size(); j++)
				{
					memcpy(transferMemoryLocation, &lvlData.levelMaterials[j].attrib, sizeof(H2B::ATTRIBUTES));
					transferMemoryLocation += sizeof(H2B::ATTRIBUTES);
				}
				materialStrdBuffer[i]->Unmap(0, nullptr);

				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.Buffer.NumElements = lvlData.levelMaterials.size();
				srvDesc.Buffer.StructureByteStride = sizeof(H2B::ATTRIBUTES);
				srvDesc.Buffer.FirstElement = 0;
				srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

				creator->CreateShaderResourceView(materialStrdBuffer[i].Get(), &srvDesc, handle);
				handle.Offset(1, sizeof(lvlData.levelMaterials));
			}
		}

		void InitializeDescriptorHeap(ID3D12Device* creator)
		{
			UINT numberOfStructuredBuffers = maxActiveFrames * 2;
			UINT numberOfDescriptors = numberOfStructuredBuffers;

			D3D12_DESCRIPTOR_HEAP_DESC cBufferHeapDesc = {};
			cBufferHeapDesc.NumDescriptors = numberOfDescriptors;
			cBufferHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			cBufferHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			creator->CreateDescriptorHeap(&cBufferHeapDesc, IID_PPV_ARGS(descriptorHeap.ReleaseAndGetAddressOf()));
		}

		void InitializeGraphics()
		{
			ID3D12Device* creator;
			d3d.GetDevice((void**)&creator);
			InitializeVertexBuffer(creator);
			InitializeIndexBuffer(creator);

			transformStrdBuffer.resize(maxActiveFrames);
			materialStrdBuffer.resize(maxActiveFrames);
			InitializeDescriptorHeap(creator);
			InitializeStructuredBuffersAndViews(creator);

			InitializeGraphicsPipeline(creator);

			// free temporary handle
			creator->Release();
		}

		void InitializeViewMatrix()
		{
			// Hard Coded
			//TODO: Make data driven
			GW::MATH::GVECTORF eye = { 0.25f, 6.5f, -0.25f, 0 };
			GW::MATH::GVECTORF at = { 0, 0, 0, 0 };
			GW::MATH::GVECTORF up = { 0, 1, 0, 0 };
			GW::MATH::GMatrix::LookAtLHF(eye, at, up, viewMatrix);
		}

		void InitializeProjectionMatrix()
		{
			float aspectRatio;
			d3d.GetAspectRatio(aspectRatio);
			GW::MATH::GMatrix::ProjectionDirectXLHF(G_DEGREE_TO_RADIAN_F(65), aspectRatio, 0.1f, 100, projectionMatrix);
		}

		void InitializeVertexBuffer(ID3D12Device* creator)
		{
			CreateVertexBuffer(creator, sizeof(H2B::VERTEX) * lvlData.levelVertices.size());
			WriteToVertexBuffer(lvlData.levelVertices.data(), sizeof(H2B::VERTEX) * lvlData.levelVertices.size());
			CreateVertexView(sizeof(H2B::VERTEX), sizeof(H2B::VERTEX) * lvlData.levelVertices.size());
		}

		void CreateVertexBuffer(ID3D12Device* creator, unsigned int sizeInBytes)
		{
			creator->CreateCommittedResource( // using UPLOAD heap for simplicity
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // DEFAULT recommend  
				D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(sizeInBytes),
				D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(vertexBuffer.ReleaseAndGetAddressOf()));
		}

		void WriteToVertexBuffer(const void* dataToWrite, unsigned int sizeInBytes)
		{
			UINT8* transferMemoryLocation;
			vertexBuffer->Map(0, &CD3DX12_RANGE(0, 0), reinterpret_cast<void**>(&transferMemoryLocation));
			memcpy(transferMemoryLocation, dataToWrite, sizeInBytes);
			vertexBuffer->Unmap(0, nullptr);
		}

		void CreateVertexView(unsigned int strideInBytes, unsigned int sizeInBytes)
		{
			vertexView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
			vertexView.StrideInBytes = strideInBytes;
			vertexView.SizeInBytes = sizeInBytes;
		}

		void InitializeIndexBuffer(ID3D12Device* creator)
		{
			CreateIndexBuffer(creator, sizeof(unsigned) * lvlData.levelIndices.size());
			WriteToIndexBuffer(lvlData.levelIndices.data(), sizeof(unsigned) * lvlData.levelIndices.size());
			CreateIndexView(sizeof(unsigned) * lvlData.levelIndices.size());
		}

		void CreateIndexBuffer(ID3D12Device* creator, unsigned int sizeInBytes)
		{
			creator->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(sizeInBytes),
				D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(indexBuffer.GetAddressOf()));
		}

		void WriteToIndexBuffer(const void* dataToWrite, unsigned int sizeInBytes)
		{
			UINT8* transferMemoryLocation;
			indexBuffer->Map(0, &CD3DX12_RANGE(0, 0), reinterpret_cast<void**>(&transferMemoryLocation));
			memcpy(transferMemoryLocation, dataToWrite, sizeInBytes);
			indexBuffer->Unmap(0, nullptr);
		}

		void CreateIndexView(unsigned int sizeInBytes)
		{
			indexView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
			indexView.Format = DXGI_FORMAT_R32_UINT;
			indexView.SizeInBytes = sizeInBytes;
		}

		void InitializeSceneDataForGPU()
		{
			//Scene Variables that currently Don't change throughout the program
			sceneDataForGPU.sunColor = sunLightColor;
			sceneDataForGPU.sunDirection = sunLightDir;
			sceneDataForGPU.sunAmbiet = sunLightAmbient;

			//Transform Init
			for (int i = 0; i < lvlData.levelTransforms.size(); i++)
			{
				transformsForGPU.push_back(lvlData.levelTransforms[i]);
			}
		}

		void UpdateTransformsForGPU(int curFrameBufferIndex)
		{
			UINT8* transferMemoryLocation = nullptr;
			transformStrdBuffer[curFrameBufferIndex]->Map(0, &CD3DX12_RANGE(0, 0), reinterpret_cast<void**>(&transferMemoryLocation));
			memcpy(transferMemoryLocation, transformsForGPU.data(), sizeof(GW::MATH::GMATRIXF) * transformsForGPU.size());
			transformStrdBuffer[curFrameBufferIndex]->Unmap(0, nullptr);

		}

		bool SetupDrawcalls();
	};	
}

#endif