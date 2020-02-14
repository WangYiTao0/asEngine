#include <asEngine.h>
#include "MeshWindow.h"

#include <sstream>

using namespace std;
namespace as
{
	using namespace asECS;
	using namespace asScene;

	MeshWindow::MeshWindow(asGUI* gui) : GUI(gui)
	{
		assert(GUI && "Invalid GUI!");

		float screenW = (float)asRenderer::GetDevice()->GetScreenWidth();
		float screenH = (float)asRenderer::GetDevice()->GetScreenHeight();


		meshWindow = new asWindow(GUI, "Mesh Window");
		meshWindow->SetSize(XMFLOAT2(800, 700));
		GUI->AddWidget(meshWindow);

		float x = 200;
		float y = 0;
		float step = 35;

		meshInfoLabel = new asLabel("Mesh Info");
		meshInfoLabel->SetPos(XMFLOAT2(x, y += step));
		meshInfoLabel->SetSize(XMFLOAT2(400, 150));
		meshWindow->AddWidget(meshInfoLabel);

		y += 160;

		doubleSidedCheckBox = new asCheckBox("Double Sided: ");
		doubleSidedCheckBox->SetTooltip("If enabled, the inside of the mesh will be visible.");
		doubleSidedCheckBox->SetPos(XMFLOAT2(x, y += step));
		doubleSidedCheckBox->OnClick([&](asEventArgs args) {
			MeshComponent* mesh = asScene::GetScene().meshes.GetComponent(entity);
			if (mesh != nullptr)
			{
				mesh->SetDoubleSided(args.bValue);
			}
			});
		meshWindow->AddWidget(doubleSidedCheckBox);

		//softbodyCheckBox = new asCheckBox("Soft body: ");
		//softbodyCheckBox->SetTooltip("Enable soft body simulation.");
		//softbodyCheckBox->SetPos(XMFLOAT2(x, y += step));
		//softbodyCheckBox->OnClick([&](asEventArgs args) {

		//	Scene& scene = asScene::GetScene();
		//	SoftBodyPhysicsComponent* physicscomponent = scene.softbodies.GetComponent(entity);

		//	if (args.bValue)
		//	{
		//		if (physicscomponent == nullptr)
		//		{
		//			SoftBodyPhysicsComponent& softbody = scene.softbodies.Create(entity);
		//			softbody.friction = frictionSlider->GetValue();
		//			softbody.mass = massSlider->GetValue();
		//		}
		//	}
		//	else
		//	{
		//		if (physicscomponent != nullptr)
		//		{
		//			scene.softbodies.Remove(entity);
		//		}
		//	}

		//	});
		//meshWindow->AddWidget(softbodyCheckBox);

		//massSlider = new asSlider(0, 10, 0, 100000, "Mass: ");
		//massSlider->SetTooltip("Set the mass amount for the physics engine.");
		//massSlider->SetSize(XMFLOAT2(100, 30));
		//massSlider->SetPos(XMFLOAT2(x, y += step));
		//massSlider->OnSlide([&](asEventArgs args) {
		//	SoftBodyPhysicsComponent* physicscomponent = asScene::GetScene().softbodies.GetComponent(entity);
		//	if (physicscomponent != nullptr)
		//	{
		//		physicscomponent->mass = args.fValue;
		//	}
		//	});
		//meshWindow->AddWidget(massSlider);

		//frictionSlider = new asSlider(0, 2, 0, 100000, "Friction: ");
		//frictionSlider->SetTooltip("Set the friction amount for the physics engine.");
		//frictionSlider->SetSize(XMFLOAT2(100, 30));
		//frictionSlider->SetPos(XMFLOAT2(x, y += step));
		//frictionSlider->OnSlide([&](asEventArgs args) {
		//	SoftBodyPhysicsComponent* physicscomponent = asScene::GetScene().softbodies.GetComponent(entity);
		//	if (physicscomponent != nullptr)
		//	{
		//		physicscomponent->friction = args.fValue;
		//	}
		//	});
		//meshWindow->AddWidget(frictionSlider);

		impostorCreateButton = new asButton("Create Impostor");
		impostorCreateButton->SetTooltip("Create an impostor image of the mesh. The mesh will be replaced by this image when far away, to render faster.");
		impostorCreateButton->SetSize(XMFLOAT2(240, 30));
		impostorCreateButton->SetPos(XMFLOAT2(x - 50, y += step));
		impostorCreateButton->OnClick([&](asEventArgs args) {
			MeshComponent* mesh = asScene::GetScene().meshes.GetComponent(entity);
			if (mesh != nullptr)
			{
				Scene& scene = asScene::GetScene();
				scene.impostors.Create(entity).swapInDistance = impostorDistanceSlider->GetValue();
			}
			});
		meshWindow->AddWidget(impostorCreateButton);

		impostorDistanceSlider = new asSlider(0, 1000, 100, 10000, "Impostor Distance: ");
		impostorDistanceSlider->SetTooltip("Assign the distance where the mesh geometry should be switched to the impostor image.");
		impostorDistanceSlider->SetSize(XMFLOAT2(100, 30));
		impostorDistanceSlider->SetPos(XMFLOAT2(x, y += step));
		impostorDistanceSlider->OnSlide([&](asEventArgs args) {
			ImpostorComponent* impostor = asScene::GetScene().impostors.GetComponent(entity);
			if (impostor != nullptr)
			{
				impostor->swapInDistance = args.fValue;
			}
			});
		meshWindow->AddWidget(impostorDistanceSlider);

		tessellationFactorSlider = new asSlider(0, 16, 0, 10000, "Tessellation Factor: ");
		tessellationFactorSlider->SetTooltip("Set the dynamic tessellation amount. Tessellation should be enabled in the Renderer window and your GPU must support it!");
		tessellationFactorSlider->SetSize(XMFLOAT2(100, 30));
		tessellationFactorSlider->SetPos(XMFLOAT2(x, y += step));
		tessellationFactorSlider->OnSlide([&](asEventArgs args) {
			MeshComponent* mesh = asScene::GetScene().meshes.GetComponent(entity);
			if (mesh != nullptr)
			{
				mesh->tessellationFactor = args.fValue;
			}
			});
		meshWindow->AddWidget(tessellationFactorSlider);

		flipCullingButton = new asButton("Flip Culling");
		flipCullingButton->SetTooltip("Flip faces to reverse triangle culling order.");
		flipCullingButton->SetSize(XMFLOAT2(240, 30));
		flipCullingButton->SetPos(XMFLOAT2(x - 50, y += step));
		flipCullingButton->OnClick([&](asEventArgs args) {
			MeshComponent* mesh = asScene::GetScene().meshes.GetComponent(entity);
			if (mesh != nullptr)
			{
				mesh->FlipCulling();
				SetEntity(entity);
			}
			});
		meshWindow->AddWidget(flipCullingButton);

		flipNormalsButton = new asButton("Flip Normals");
		flipNormalsButton->SetTooltip("Flip surface normals.");
		flipNormalsButton->SetSize(XMFLOAT2(240, 30));
		flipNormalsButton->SetPos(XMFLOAT2(x - 50, y += step));
		flipNormalsButton->OnClick([&](asEventArgs args) {
			MeshComponent* mesh = asScene::GetScene().meshes.GetComponent(entity);
			if (mesh != nullptr)
			{
				mesh->FlipNormals();
				SetEntity(entity);
			}
			});
		meshWindow->AddWidget(flipNormalsButton);

		computeNormalsSmoothButton = new asButton("Compute Normals [SMOOTH]");
		computeNormalsSmoothButton->SetTooltip("Compute surface normals of the mesh. Resulting normals will be unique per vertex.");
		computeNormalsSmoothButton->SetSize(XMFLOAT2(240, 30));
		computeNormalsSmoothButton->SetPos(XMFLOAT2(x - 50, y += step));
		computeNormalsSmoothButton->OnClick([&](asEventArgs args) {
			MeshComponent* mesh = asScene::GetScene().meshes.GetComponent(entity);
			if (mesh != nullptr)
			{
				mesh->ComputeNormals(true);
				SetEntity(entity);
			}
			});
		meshWindow->AddWidget(computeNormalsSmoothButton);

		computeNormalsHardButton = new asButton("Compute Normals [HARD]");
		computeNormalsHardButton->SetTooltip("Compute surface normals of the mesh. Resulting normals will be unique per face.");
		computeNormalsHardButton->SetSize(XMFLOAT2(240, 30));
		computeNormalsHardButton->SetPos(XMFLOAT2(x - 50, y += step));
		computeNormalsHardButton->OnClick([&](asEventArgs args) {
			MeshComponent* mesh = asScene::GetScene().meshes.GetComponent(entity);
			if (mesh != nullptr)
			{
				mesh->ComputeNormals(false);
				SetEntity(entity);
			}
			});
		meshWindow->AddWidget(computeNormalsHardButton);

		recenterButton = new asButton("Recenter");
		recenterButton->SetTooltip("Recenter mesh to AABB center.");
		recenterButton->SetSize(XMFLOAT2(240, 30));
		recenterButton->SetPos(XMFLOAT2(x - 50, y += step));
		recenterButton->OnClick([&](asEventArgs args) {
			MeshComponent* mesh = asScene::GetScene().meshes.GetComponent(entity);
			if (mesh != nullptr)
			{
				mesh->Recenter();
				SetEntity(entity);
			}
			});
		meshWindow->AddWidget(recenterButton);

		recenterToBottomButton = new asButton("RecenterToBottom");
		recenterToBottomButton->SetTooltip("Recenter mesh to AABB bottom.");
		recenterToBottomButton->SetSize(XMFLOAT2(240, 30));
		recenterToBottomButton->SetPos(XMFLOAT2(x - 50, y += step));
		recenterToBottomButton->OnClick([&](asEventArgs args) {
			MeshComponent* mesh = asScene::GetScene().meshes.GetComponent(entity);
			if (mesh != nullptr)
			{
				mesh->RecenterToBottom();
				SetEntity(entity);
			}
			});
		meshWindow->AddWidget(recenterToBottomButton);




		meshWindow->Translate(XMFLOAT3(screenW - 910, 520, 0));
		meshWindow->SetVisible(false);

		SetEntity(INVALID_ENTITY);
	}


	MeshWindow::~MeshWindow()
	{
		meshWindow->RemoveWidgets(true);
		GUI->RemoveWidget(meshWindow);
		SAFE_DELETE(meshWindow);
	}

	void MeshWindow::SetEntity(Entity entity)
	{
		this->entity = entity;

		Scene& scene = asScene::GetScene();

		const MeshComponent* mesh = scene.meshes.GetComponent(entity);

		if (mesh != nullptr)
		{
			const NameComponent& name = *scene.names.GetComponent(entity);

			stringstream ss("");
			ss << "Mesh name: " << name.name << endl;
			ss << "Vertex count: " << mesh->vertex_positions.size() << endl;
			ss << "Index count: " << mesh->indices.size() << endl;
			ss << "Subset count: " << mesh->subsets.size() << endl;
			ss << endl << "Vertex buffers: ";
			if (mesh->vertexBuffer_POS != nullptr) ss << "position; ";
			if (mesh->vertexBuffer_UV0 != nullptr) ss << "uvset_0; ";
			if (mesh->vertexBuffer_UV1 != nullptr) ss << "uvset_1; ";
			if (mesh->vertexBuffer_ATL != nullptr) ss << "atlas; ";
			if (mesh->vertexBuffer_COL != nullptr) ss << "color; ";
			if (mesh->vertexBuffer_PRE != nullptr) ss << "prevPos; ";
			if (mesh->vertexBuffer_BON != nullptr) ss << "bone; ";
			if (mesh->streamoutBuffer_POS != nullptr) ss << "streamout; ";
			meshInfoLabel->SetText(ss.str());

			doubleSidedCheckBox->SetCheck(mesh->IsDoubleSided());

			const ImpostorComponent* impostor = scene.impostors.GetComponent(entity);
			if (impostor != nullptr)
			{
				impostorDistanceSlider->SetValue(impostor->swapInDistance);
			}
			tessellationFactorSlider->SetValue(mesh->GetTessellationFactor());

			softbodyCheckBox->SetCheck(false);

			SoftBodyPhysicsComponent* physicscomponent = asScene::GetScene().softbodies.GetComponent(entity);
			if (physicscomponent != nullptr)
			{
				softbodyCheckBox->SetCheck(true);
				massSlider->SetValue(physicscomponent->mass);
				frictionSlider->SetValue(physicscomponent->friction);
			}
			meshWindow->SetEnabled(true);
		}
		else
		{
			meshInfoLabel->SetText("Select a mesh...");
			meshWindow->SetEnabled(false);
		}
	}
}
