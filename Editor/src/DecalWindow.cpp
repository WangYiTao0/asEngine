#include <asEngine.h>
#include "DecalWindow.h"

namespace as
{
	using namespace asECS;
	using namespace asScene;


	DecalWindow::DecalWindow(asGUI* gui) : GUI(gui)
	{
		assert(GUI && "Invalid GUI!");

		float screenW = (float)asRenderer::GetDevice()->GetScreenWidth();
		float screenH = (float)asRenderer::GetDevice()->GetScreenHeight();

		decalWindow = new asWindow(GUI, "Decal Window");
		decalWindow->SetSize(XMFLOAT2(400, 300));
		GUI->AddWidget(decalWindow);

		float x = 200;
		float y = 0;

		decalNameField = new asTextInputField("MaterialName");
		decalNameField->SetPos(XMFLOAT2(10, 30));
		decalNameField->SetSize(XMFLOAT2(300, 20));
		decalNameField->OnInputAccepted([&](asEventArgs args) {
			NameComponent* name = asScene::GetScene().names.GetComponent(entity);
			if (name != nullptr)
			{
				*name = args.sValue;
			}
			});
		decalWindow->AddWidget(decalNameField);

		decalWindow->Translate(XMFLOAT3(30, 30, 0));
		decalWindow->SetVisible(false);

		SetEntity(INVALID_ENTITY);
	}


	DecalWindow::~DecalWindow()
	{
		decalWindow->RemoveWidgets(true);
		GUI->RemoveWidget(decalWindow);
		SAFE_DELETE(decalWindow);
	}

	void DecalWindow::SetEntity(Entity entity)
	{
		this->entity = entity;

		Scene& scene = asScene::GetScene();
		const DecalComponent* decal = scene.decals.GetComponent(entity);

		if (decal != nullptr)
		{
			const NameComponent& name = *scene.names.GetComponent(entity);

			decalNameField->SetValue(name.name);
			decalNameField->SetEnabled(true);
		}
		else
		{
			decalNameField->SetValue("No decal selected");
			decalNameField->SetEnabled(false);
		}
	}
}