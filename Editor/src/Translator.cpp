#include <asEngine.h>
#include "Translator.h"

namespace as
{
	using namespace asGraphics;
	using namespace asECS;
	using namespace asScene;

	PipelineState pso_solidpart;
	PipelineState pso_wirepart;
	std::unique_ptr<asGraphics::GPUBuffer> vertexBuffer_Axis;
	std::unique_ptr<asGraphics::GPUBuffer> vertexBuffer_Plane;
	std::unique_ptr<asGraphics::GPUBuffer> vertexBuffer_Origin;
	UINT vertexCount_Axis = 0;
	UINT vertexCount_Plane = 0;
	UINT vertexCount_Origin = 0;

	void Translator::LoadShaders()
	{
		GraphicsDevice* device = asRenderer::GetDevice();

		{
			PipelineStateDesc desc;

			desc.vs = asRenderer::GetVertexShader(VSTYPE_LINE);
			desc.ps = asRenderer::GetPixelShader(PSTYPE_LINE);
			desc.il = asRenderer::GetVertexLayout(VLTYPE_LINE);
			desc.dss = asRenderer::GetDepthStencilState(DSSTYPE_XRAY);
			desc.rs = asRenderer::GetRasterizerState(RSTYPE_DOUBLESIDED);
			desc.bs = asRenderer::GetBlendState(BSTYPE_ADDITIVE);
			desc.pt = TRIANGLELIST;

			device->CreatePipelineState(&desc, &pso_solidpart);
		}

		{
			PipelineStateDesc desc;

			desc.vs = asRenderer::GetVertexShader(VSTYPE_LINE);
			desc.ps = asRenderer::GetPixelShader(PSTYPE_LINE);
			desc.il = asRenderer::GetVertexLayout(VLTYPE_LINE);
			desc.dss = asRenderer::GetDepthStencilState(DSSTYPE_XRAY);
			desc.rs = asRenderer::GetRasterizerState(RSTYPE_WIRE_DOUBLESIDED_SMOOTH);
			desc.bs = asRenderer::GetBlendState(BSTYPE_TRANSPARENT);
			desc.pt = LINELIST;

			device->CreatePipelineState(&desc, &pso_wirepart);
		}
	}

	Translator::Translator()
	{
		entityID = CreateEntity();

		prevPointer = XMFLOAT4(0, 0, 0, 0);

		XMStoreFloat4x4(&dragStart, XMMatrixIdentity());
		XMStoreFloat4x4(&dragEnd, XMMatrixIdentity());

		dragging = false;
		dragStarted = false;
		dragEnded = false;

		enabled = true;

		state = TRANSLATOR_IDLE;

		dist = 1;

		isTranslator = true;
		isScalator = false;
		isRotator = false;


		GraphicsDevice* device = asRenderer::GetDevice();

		if (vertexBuffer_Axis == nullptr)
		{
			{
				XMFLOAT4 verts[] = {
					XMFLOAT4(0,0,0,1), XMFLOAT4(1,1,1,1),
					XMFLOAT4(3,0,0,1), XMFLOAT4(1,1,1,1),
				};
				vertexCount_Axis = arraysize(verts) / 2;

				GPUBufferDesc bd;
				bd.Usage = USAGE_DEFAULT;
				bd.ByteWidth = sizeof(verts);
				bd.BindFlags = BIND_VERTEX_BUFFER;
				bd.CPUAccessFlags = 0;

				SubresourceData InitData;
				InitData.pSysMem = verts;

				vertexBuffer_Axis.reset(new GPUBuffer);
				device->CreateBuffer(&bd, &InitData, vertexBuffer_Axis.get());
			}
		}

		if (vertexBuffer_Plane == nullptr)
		{
			{
				XMFLOAT4 verts[] = {
					XMFLOAT4(0,0,0,1), XMFLOAT4(1,1,1,1),
					XMFLOAT4(1,0,0,1), XMFLOAT4(1,1,1,1),
					XMFLOAT4(1,1,0,1), XMFLOAT4(1,1,1,1),

					XMFLOAT4(0,0,0,1), XMFLOAT4(1,1,1,1),
					XMFLOAT4(1,1,0,1), XMFLOAT4(1,1,1,1),
					XMFLOAT4(0,1,0,1), XMFLOAT4(1,1,1,1),
				};
				vertexCount_Plane = arraysize(verts) / 2;

				GPUBufferDesc bd;
				bd.Usage = USAGE_DEFAULT;
				bd.ByteWidth = sizeof(verts);
				bd.BindFlags = BIND_VERTEX_BUFFER;
				bd.CPUAccessFlags = 0;

				SubresourceData InitData;
				InitData.pSysMem = verts;
				vertexBuffer_Plane.reset(new GPUBuffer);
				device->CreateBuffer(&bd, &InitData, vertexBuffer_Plane.get());
			}
		}

		if (vertexBuffer_Origin == nullptr)
		{
			{
				float edge = 0.2f;
				XMFLOAT4 verts[] = {
					XMFLOAT4(-edge,edge,edge,1),   XMFLOAT4(1,1,1,1),
					XMFLOAT4(-edge,-edge,edge,1),  XMFLOAT4(1,1,1,1),
					XMFLOAT4(-edge,-edge,-edge,1), XMFLOAT4(1,1,1,1),
					XMFLOAT4(edge,edge,edge,1),	   XMFLOAT4(1,1,1,1),
					XMFLOAT4(edge,-edge,edge,1),   XMFLOAT4(1,1,1,1),
					XMFLOAT4(-edge,-edge,edge,1),  XMFLOAT4(1,1,1,1),
					XMFLOAT4(edge,edge,-edge,1),   XMFLOAT4(1,1,1,1),
					XMFLOAT4(edge,-edge,-edge,1),  XMFLOAT4(1,1,1,1),
					XMFLOAT4(edge,-edge,edge,1),   XMFLOAT4(1,1,1,1),
					XMFLOAT4(-edge,edge,-edge,1),  XMFLOAT4(1,1,1,1),
					XMFLOAT4(-edge,-edge,-edge,1), XMFLOAT4(1,1,1,1),
					XMFLOAT4(edge,-edge,-edge,1),  XMFLOAT4(1,1,1,1),
					XMFLOAT4(-edge,-edge,edge,1),  XMFLOAT4(1,1,1,1),
					XMFLOAT4(edge,-edge,edge,1),   XMFLOAT4(1,1,1,1),
					XMFLOAT4(edge,-edge,-edge,1),  XMFLOAT4(1,1,1,1),
					XMFLOAT4(edge,edge,edge,1),	   XMFLOAT4(1,1,1,1),
					XMFLOAT4(-edge,edge,edge,1),   XMFLOAT4(1,1,1,1),
					XMFLOAT4(-edge,edge,-edge,1),  XMFLOAT4(1,1,1,1),
					XMFLOAT4(-edge,edge,-edge,1),  XMFLOAT4(1,1,1,1),
					XMFLOAT4(-edge,edge,edge,1),   XMFLOAT4(1,1,1,1),
					XMFLOAT4(-edge,-edge,-edge,1), XMFLOAT4(1,1,1,1),
					XMFLOAT4(-edge,edge,edge,1),   XMFLOAT4(1,1,1,1),
					XMFLOAT4(edge,edge,edge,1),	   XMFLOAT4(1,1,1,1),
					XMFLOAT4(-edge,-edge,edge,1),  XMFLOAT4(1,1,1,1),
					XMFLOAT4(edge,edge,edge,1),	   XMFLOAT4(1,1,1,1),
					XMFLOAT4(edge,edge,-edge,1),   XMFLOAT4(1,1,1,1),
					XMFLOAT4(edge,-edge,edge,1),   XMFLOAT4(1,1,1,1),
					XMFLOAT4(edge,edge,-edge,1),   XMFLOAT4(1,1,1,1),
					XMFLOAT4(-edge,edge,-edge,1),  XMFLOAT4(1,1,1,1),
					XMFLOAT4(edge,-edge,-edge,1),  XMFLOAT4(1,1,1,1),
					XMFLOAT4(-edge,-edge,-edge,1), XMFLOAT4(1,1,1,1),
					XMFLOAT4(-edge,-edge,edge,1),  XMFLOAT4(1,1,1,1),
					XMFLOAT4(edge,-edge,-edge,1),  XMFLOAT4(1,1,1,1),
					XMFLOAT4(edge,edge,-edge,1),   XMFLOAT4(1,1,1,1),
					XMFLOAT4(edge,edge,edge,1),	   XMFLOAT4(1,1,1,1),
					XMFLOAT4(-edge,edge,-edge,1),  XMFLOAT4(1,1,1,1),
				};
				vertexCount_Origin = arraysize(verts) / 2;

				GPUBufferDesc bd;
				bd.Usage = USAGE_DEFAULT;
				bd.ByteWidth = sizeof(verts);
				bd.BindFlags = BIND_VERTEX_BUFFER;
				bd.CPUAccessFlags = 0;

				SubresourceData InitData;
				InitData.pSysMem = verts;
				vertexBuffer_Origin.reset(new GPUBuffer);
				device->CreateBuffer(&bd, &InitData, vertexBuffer_Origin.get());
			}
		}
	}

	Translator::~Translator()
	{
	}

	void Translator::Update()
	{
		Scene& scene = asScene::GetScene();

		if (!scene.transforms.Contains(entityID))
		{
			return;
		}

		TransformComponent& transform = *scene.transforms.GetComponent(entityID);

		dragStarted = false;
		dragEnded = false;

		XMFLOAT4 pointer = asInput::GetPointer();
		const CameraComponent& cam = asRenderer::GetCamera();
		XMVECTOR pos = transform.GetPositionV();

		if (enabled)
		{

			if (!dragging)
			{
				XMMATRIX P = cam.GetProjection();
				XMMATRIX V = cam.GetView();
				XMMATRIX W = XMMatrixIdentity();

				dist = asMath::Distance(transform.GetPosition(), cam.Eye) * 0.05f;

				XMVECTOR o, x, y, z, p, xy, xz, yz;

				o = pos;
				x = o + XMVectorSet(3, 0, 0, 0) * dist;
				y = o + XMVectorSet(0, 3, 0, 0) * dist;
				z = o + XMVectorSet(0, 0, 3, 0) * dist;
				p = XMLoadFloat4(&pointer);
				xy = o + XMVectorSet(0.5f, 0.5f, 0, 0) * dist;
				xz = o + XMVectorSet(0.5f, 0, 0.5f, 0) * dist;
				yz = o + XMVectorSet(0, 0.5f, 0.5f, 0) * dist;


				o = XMVector3Project(o, 0, 0, cam.width, cam.height, 0, 1, P, V, W);
				x = XMVector3Project(x, 0, 0, cam.width, cam.height, 0, 1, P, V, W);
				y = XMVector3Project(y, 0, 0, cam.width, cam.height, 0, 1, P, V, W);
				z = XMVector3Project(z, 0, 0, cam.width, cam.height, 0, 1, P, V, W);
				xy = XMVector3Project(xy, 0, 0, cam.width, cam.height, 0, 1, P, V, W);
				xz = XMVector3Project(xz, 0, 0, cam.width, cam.height, 0, 1, P, V, W);
				yz = XMVector3Project(yz, 0, 0, cam.width, cam.height, 0, 1, P, V, W);

				XMVECTOR oDisV = XMVector3Length(o - p);
				XMVECTOR xyDisV = XMVector3Length(xy - p);
				XMVECTOR xzDisV = XMVector3Length(xz - p);
				XMVECTOR yzDisV = XMVector3Length(yz - p);

				float xDis = asMath::GetPointSegmentDistance(p, o, x);
				float yDis = asMath::GetPointSegmentDistance(p, o, y);
				float zDis = asMath::GetPointSegmentDistance(p, o, z);
				float oDis = XMVectorGetX(oDisV);
				float xyDis = XMVectorGetX(xyDisV);
				float xzDis = XMVectorGetX(xzDisV);
				float yzDis = XMVectorGetX(yzDisV);

				if (oDis < 10)
				{
					state = TRANSLATOR_XYZ;
				}
				else if (xyDis < 20)
				{
					state = TRANSLATOR_XY;
				}
				else if (xzDis < 20)
				{
					state = TRANSLATOR_XZ;
				}
				else if (yzDis < 20)
				{
					state = TRANSLATOR_YZ;
				}
				else if (xDis < 10)
				{
					state = TRANSLATOR_X;
				}
				else if (yDis < 10)
				{
					state = TRANSLATOR_Y;
				}
				else if (zDis < 10)
				{
					state = TRANSLATOR_Z;
				}
				else if (!dragging)
				{
					state = TRANSLATOR_IDLE;
				}
			}

			if (dragging || (state != TRANSLATOR_IDLE && asInput::Down(asInput::MOUSE_BUTTON_LEFT)))
			{
				XMVECTOR plane, planeNormal;
				if (state == TRANSLATOR_X)
				{
					XMVECTOR axis = XMVectorSet(1, 0, 0, 0);
					XMVECTOR wrong = XMVector3Cross(cam.GetAt(), axis);
					planeNormal = XMVector3Cross(wrong, axis);
				}
				else if (state == TRANSLATOR_Y)
				{
					XMVECTOR axis = XMVectorSet(0, 1, 0, 0);
					XMVECTOR wrong = XMVector3Cross(cam.GetAt(), axis);
					planeNormal = XMVector3Cross(wrong, axis);
				}
				else if (state == TRANSLATOR_Z)
				{
					XMVECTOR axis = XMVectorSet(0, 0, 1, 0);
					XMVECTOR wrong = XMVector3Cross(cam.GetAt(), axis);
					planeNormal = XMVector3Cross(wrong, axis);
				}
				else if (state == TRANSLATOR_XY)
				{
					planeNormal = XMVectorSet(0, 0, 1, 0);
				}
				else if (state == TRANSLATOR_XZ)
				{
					planeNormal = XMVectorSet(0, 1, 0, 0);
				}
				else if (state == TRANSLATOR_YZ)
				{
					planeNormal = XMVectorSet(1, 0, 0, 0);
				}
				else
				{
					// xyz
					planeNormal = cam.GetAt();
				}
				plane = XMPlaneFromPointNormal(pos, XMVector3Normalize(planeNormal));

				RAY ray = asRenderer::GetPickRay((long)pointer.x, (long)pointer.y);
				XMVECTOR rayOrigin = XMLoadFloat3(&ray.origin);
				XMVECTOR rayDir = XMLoadFloat3(&ray.direction);
				XMVECTOR intersection = XMPlaneIntersectLine(plane, rayOrigin, rayOrigin + rayDir * cam.zFarP);

				ray = asRenderer::GetPickRay((long)prevPointer.x, (long)prevPointer.y);
				rayOrigin = XMLoadFloat3(&ray.origin);
				rayDir = XMLoadFloat3(&ray.direction);
				XMVECTOR intersectionPrev = XMPlaneIntersectLine(plane, rayOrigin, rayOrigin + rayDir * cam.zFarP);

				XMVECTOR deltaV;
				if (state == TRANSLATOR_X)
				{
					XMVECTOR A = pos, B = pos + XMVectorSet(1, 0, 0, 0);
					XMVECTOR P = asMath::GetClosestPointToLine(A, B, intersection);
					XMVECTOR PPrev = asMath::GetClosestPointToLine(A, B, intersectionPrev);
					deltaV = P - PPrev;
				}
				else if (state == TRANSLATOR_Y)
				{
					XMVECTOR A = pos, B = pos + XMVectorSet(0, 1, 0, 0);
					XMVECTOR P = asMath::GetClosestPointToLine(A, B, intersection);
					XMVECTOR PPrev = asMath::GetClosestPointToLine(A, B, intersectionPrev);
					deltaV = P - PPrev;
				}
				else if (state == TRANSLATOR_Z)
				{
					XMVECTOR A = pos, B = pos + XMVectorSet(0, 0, 1, 0);
					XMVECTOR P = asMath::GetClosestPointToLine(A, B, intersection);
					XMVECTOR PPrev = asMath::GetClosestPointToLine(A, B, intersectionPrev);
					deltaV = P - PPrev;
				}
				else
				{
					deltaV = intersection - intersectionPrev;

					if (isScalator)
					{
						deltaV = XMVectorSplatY(deltaV);
					}
				}

				XMFLOAT3 delta;
				if (isRotator)
				{
					deltaV /= XMVector3Length(intersection - rayOrigin);
					deltaV *= XM_2PI;
				}
				XMStoreFloat3(&delta, deltaV);


				XMMATRIX transf = XMMatrixIdentity();

				if (isTranslator)
				{
					transf *= XMMatrixTranslation(delta.x, delta.y, delta.z);
				}
				if (isRotator)
				{
					transf *= XMMatrixRotationRollPitchYaw(delta.x, delta.y, delta.z);
				}
				if (isScalator)
				{
					XMFLOAT3 scale = transform.GetScale();
					transf *= XMMatrixScaling((1.0f / scale.x) * (scale.x + delta.x), (1.0f / scale.y) * (scale.y + delta.y), (1.0f / scale.z) * (scale.z + delta.z));
				}

				transform.MatrixTransform(transf);

				if (!dragging)
				{
					dragStarted = true;
					dragStart = transform.world;
				}

				dragging = true;
			}

			if (!asInput::Down(asInput::MOUSE_BUTTON_LEFT))
			{
				if (dragging)
				{
					dragEnded = true;
					dragEnd = transform.world;
				}
				dragging = false;
			}

		}
		else
		{
			if (dragging)
			{
				dragEnded = true;
				dragEnd = transform.world;
			}
			dragging = false;
		}

		prevPointer = pointer;
	}
	void Translator::Draw(const CameraComponent& camera, CommandList cmd) const
	{
		Scene& scene = asScene::GetScene();

		if (!scene.transforms.Contains(entityID))
		{
			return;
		}

		static bool shaders_loaded = false;
		if (!shaders_loaded)
		{
			shaders_loaded = true;
			LoadShaders();
		}

		TransformComponent& transform = *scene.transforms.GetComponent(entityID);

		GraphicsDevice* device = asRenderer::GetDevice();

		device->EventBegin("Editor - Translator", cmd);

		XMMATRIX VP = camera.GetViewProjection();

		MiscCB sb;

		XMMATRIX mat = XMMatrixScaling(dist, dist, dist) * XMMatrixTranslationFromVector(transform.GetPositionV()) * VP;
		XMMATRIX matX = mat;
		XMMATRIX matY = XMMatrixRotationZ(XM_PIDIV2) * XMMatrixRotationY(XM_PIDIV2) * mat;
		XMMATRIX matZ = XMMatrixRotationY(-XM_PIDIV2) * XMMatrixRotationZ(-XM_PIDIV2) * mat;

		// Planes:
		{
			device->BindPipelineState(&pso_solidpart, cmd);
			const GPUBuffer* vbs[] = {
				vertexBuffer_Plane.get(),
			};
			const UINT strides[] = {
				sizeof(XMFLOAT4) + sizeof(XMFLOAT4),
			};
			device->BindVertexBuffers(vbs, 0, arraysize(vbs), strides, nullptr, cmd);
		}

		// xy
		XMStoreFloat4x4(&sb.g_xTransform, matX);
		sb.g_xColor = state == TRANSLATOR_XY ? XMFLOAT4(1, 1, 1, 1) : XMFLOAT4(0.2f, 0.2f, 0, 0.2f);
		device->UpdateBuffer(asRenderer::GetConstantBuffer(CBTYPE_MISC), &sb, cmd);
		device->BindConstantBuffer(VS, asRenderer::GetConstantBuffer(CBTYPE_MISC), CB_GETBINDSLOT(MiscCB), cmd);
		device->BindConstantBuffer(PS, asRenderer::GetConstantBuffer(CBTYPE_MISC), CB_GETBINDSLOT(MiscCB), cmd);
		device->Draw(vertexCount_Plane, 0, cmd);

		// xz
		XMStoreFloat4x4(&sb.g_xTransform, matZ);
		sb.g_xColor = state == TRANSLATOR_XZ ? XMFLOAT4(1, 1, 1, 1) : XMFLOAT4(0.2f, 0.2f, 0, 0.2f);
		device->UpdateBuffer(asRenderer::GetConstantBuffer(CBTYPE_MISC), &sb, cmd);
		device->BindConstantBuffer(VS, asRenderer::GetConstantBuffer(CBTYPE_MISC), CB_GETBINDSLOT(MiscCB), cmd);
		device->BindConstantBuffer(PS, asRenderer::GetConstantBuffer(CBTYPE_MISC), CB_GETBINDSLOT(MiscCB), cmd);
		device->Draw(vertexCount_Plane, 0, cmd);

		// yz
		XMStoreFloat4x4(&sb.g_xTransform, matY);
		sb.g_xColor = state == TRANSLATOR_YZ ? XMFLOAT4(1, 1, 1, 1) : XMFLOAT4(0.2f, 0.2f, 0, 0.2f);
		device->UpdateBuffer(asRenderer::GetConstantBuffer(CBTYPE_MISC), &sb, cmd);
		device->BindConstantBuffer(VS, asRenderer::GetConstantBuffer(CBTYPE_MISC), CB_GETBINDSLOT(MiscCB), cmd);
		device->BindConstantBuffer(PS, asRenderer::GetConstantBuffer(CBTYPE_MISC), CB_GETBINDSLOT(MiscCB), cmd);
		device->Draw(vertexCount_Plane, 0, cmd);

		// Lines:
		{
			device->BindPipelineState(&pso_wirepart, cmd);
			const GPUBuffer* vbs[] = {
				vertexBuffer_Axis.get(),
			};
			const UINT strides[] = {
				sizeof(XMFLOAT4) + sizeof(XMFLOAT4),
			};
			device->BindVertexBuffers(vbs, 0, arraysize(vbs), strides, nullptr, cmd);
		}

		// x
		XMStoreFloat4x4(&sb.g_xTransform, matX);
		sb.g_xColor = state == TRANSLATOR_X ? XMFLOAT4(1, 1, 1, 1) : XMFLOAT4(1, 0, 0, 1);
		device->UpdateBuffer(asRenderer::GetConstantBuffer(CBTYPE_MISC), &sb, cmd);
		device->BindConstantBuffer(VS, asRenderer::GetConstantBuffer(CBTYPE_MISC), CB_GETBINDSLOT(MiscCB), cmd);
		device->BindConstantBuffer(PS, asRenderer::GetConstantBuffer(CBTYPE_MISC), CB_GETBINDSLOT(MiscCB), cmd);
		device->Draw(vertexCount_Axis, 0, cmd);

		// y
		XMStoreFloat4x4(&sb.g_xTransform, matY);
		sb.g_xColor = state == TRANSLATOR_Y ? XMFLOAT4(1, 1, 1, 1) : XMFLOAT4(0, 1, 0, 1);
		device->UpdateBuffer(asRenderer::GetConstantBuffer(CBTYPE_MISC), &sb, cmd);
		device->BindConstantBuffer(VS, asRenderer::GetConstantBuffer(CBTYPE_MISC), CB_GETBINDSLOT(MiscCB), cmd);
		device->BindConstantBuffer(PS, asRenderer::GetConstantBuffer(CBTYPE_MISC), CB_GETBINDSLOT(MiscCB), cmd);
		device->Draw(vertexCount_Axis, 0, cmd);

		// z
		XMStoreFloat4x4(&sb.g_xTransform, matZ);
		sb.g_xColor = state == TRANSLATOR_Z ? XMFLOAT4(1, 1, 1, 1) : XMFLOAT4(0, 0, 1, 1);
		device->UpdateBuffer(asRenderer::GetConstantBuffer(CBTYPE_MISC), &sb, cmd);
		device->BindConstantBuffer(VS, asRenderer::GetConstantBuffer(CBTYPE_MISC), CB_GETBINDSLOT(MiscCB), cmd);
		device->BindConstantBuffer(PS, asRenderer::GetConstantBuffer(CBTYPE_MISC), CB_GETBINDSLOT(MiscCB), cmd);
		device->Draw(vertexCount_Axis, 0, cmd);

		// Origin:
		{
			device->BindPipelineState(&pso_solidpart, cmd);
			const GPUBuffer* vbs[] = {
				vertexBuffer_Origin.get(),
			};
			const UINT strides[] = {
				sizeof(XMFLOAT4) + sizeof(XMFLOAT4),
			};
			device->BindVertexBuffers(vbs, 0, arraysize(vbs), strides, nullptr, cmd);
			XMStoreFloat4x4(&sb.g_xTransform, mat);
			sb.g_xColor = state == TRANSLATOR_XYZ ? XMFLOAT4(1, 1, 1, 1) : XMFLOAT4(0.25f, 0.25f, 0.25f, 1);
			device->UpdateBuffer(asRenderer::GetConstantBuffer(CBTYPE_MISC), &sb, cmd);
			device->BindConstantBuffer(VS, asRenderer::GetConstantBuffer(CBTYPE_MISC), CB_GETBINDSLOT(MiscCB), cmd);
			device->BindConstantBuffer(PS, asRenderer::GetConstantBuffer(CBTYPE_MISC), CB_GETBINDSLOT(MiscCB), cmd);
			device->Draw(vertexCount_Origin, 0, cmd);
		}

		device->EventEnd(cmd);
	}

	bool Translator::IsDragStarted()
	{
		return dragStarted;
	}
	XMFLOAT4X4 Translator::GetDragStart()
	{
		return dragStart;
	}
	bool Translator::IsDragEnded()
	{
		return dragEnded;
	}
	XMFLOAT4X4 Translator::GetDragEnd()
	{
		return dragEnd;
	}
}