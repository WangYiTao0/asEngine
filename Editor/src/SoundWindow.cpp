#include <asEngine.h>
#include "SoundWindow.h"

#include <sstream>

using namespace std;

namespace as
{
	using namespace asGraphics;
	using namespace asECS;
	using namespace asScene;

	SoundWindow::SoundWindow(asGUI* gui) : GUI(gui)
	{
		assert(GUI && "Invalid GUI!");

		float screenW = (float)asRenderer::GetDevice()->GetScreenWidth();
		float screenH = (float)asRenderer::GetDevice()->GetScreenHeight();

		soundWindow = new asWindow(GUI, "Sound Window");
		soundWindow->SetSize(XMFLOAT2(440, 340));
		GUI->AddWidget(soundWindow);

		float x = 20;
		float y = 0;
		float step = 35;

		reverbComboBox = new asComboBox("Reverb: ");
		reverbComboBox->SetPos(XMFLOAT2(x + 80, y += step));
		reverbComboBox->SetSize(XMFLOAT2(180, 25));
		reverbComboBox->OnSelect([&](asEventArgs args) {
			asAudio::SetReverb((asAudio::REVERB_PRESET)args.iValue);
			});
		reverbComboBox->AddItem("DEFAULT");
		reverbComboBox->AddItem("GENERIC");
		reverbComboBox->AddItem("FOREST");
		reverbComboBox->AddItem("PADDEDCELL");
		reverbComboBox->AddItem("ROOM");
		reverbComboBox->AddItem("BATHROOM");
		reverbComboBox->AddItem("LIVINGROOM");
		reverbComboBox->AddItem("STONEROOM");
		reverbComboBox->AddItem("AUDITORIUM");
		reverbComboBox->AddItem("CONCERTHALL");
		reverbComboBox->AddItem("CAVE");
		reverbComboBox->AddItem("ARENA");
		reverbComboBox->AddItem("HANGAR");
		reverbComboBox->AddItem("CARPETEDHALLWAY");
		reverbComboBox->AddItem("HALLWAY");
		reverbComboBox->AddItem("STONECORRIDOR");
		reverbComboBox->AddItem("ALLEY");
		reverbComboBox->AddItem("CITY");
		reverbComboBox->AddItem("MOUNTAINS");
		reverbComboBox->AddItem("QUARRY");
		reverbComboBox->AddItem("PLAIN");
		reverbComboBox->AddItem("PARKINGLOT");
		reverbComboBox->AddItem("SEWERPIPE");
		reverbComboBox->AddItem("UNDERWATER");
		reverbComboBox->AddItem("SMALLROOM");
		reverbComboBox->AddItem("MEDIUMROOM");
		reverbComboBox->AddItem("LARGEROOM");
		reverbComboBox->AddItem("MEDIUMHALL");
		reverbComboBox->AddItem("LARGEHALL");
		reverbComboBox->AddItem("PLATE");
		reverbComboBox->SetTooltip("Set the global reverb setting.");
		soundWindow->AddWidget(reverbComboBox);

		y += step;

		addButton = new asButton("Add Sound");
		addButton->SetTooltip("Add a sound file to the scene.");
		addButton->SetPos(XMFLOAT2(x, y += step));
		addButton->SetSize(XMFLOAT2(80, 30));
		addButton->OnClick([&](asEventArgs args) {
			asHelper::FileDialogParams params;
			asHelper::FileDialogResult result;
			params.type = asHelper::FileDialogParams::OPEN;
			params.description = "Sound";
			params.extensions.push_back("wav");
			asHelper::FileDialog(params, result);

			if (result.ok) {
				string fileName = result.filenames.front();
				Entity entity = GetScene().Entity_CreateSound("editorSound", fileName);
			}
			});
		soundWindow->AddWidget(addButton);

		filenameLabel = new asLabel("Filename");
		filenameLabel->SetPos(XMFLOAT2(x, y += step));
		filenameLabel->SetSize(XMFLOAT2(400, 20));
		soundWindow->AddWidget(filenameLabel);

		nameField = new asTextInputField("SoundName");
		nameField->SetTooltip("Enter a sound name to identify this entity...");
		nameField->SetPos(XMFLOAT2(x, y += step));
		nameField->SetSize(XMFLOAT2(300, 20));
		nameField->OnInputAccepted([&](asEventArgs args) {
			NameComponent* name = asScene::GetScene().names.GetComponent(entity);
			if (name == nullptr)
			{
				name = &asScene::GetScene().names.Create(entity);
			}
			*name = args.sValue;
			});
		soundWindow->AddWidget(nameField);
		nameField->SetEnabled(false);

		playstopButton = new asButton("Play");
		playstopButton->SetTooltip("Play/Stop selected sound instance.");
		playstopButton->SetPos(XMFLOAT2(x, y += step));
		playstopButton->SetSize(XMFLOAT2(80, 30));
		playstopButton->OnClick([&](asEventArgs args) {
			SoundComponent* sound = GetScene().sounds.GetComponent(entity);
			if (sound != nullptr)
			{
				if (sound->IsPlaying())
				{
					sound->Stop();
					playstopButton->SetText("Play");
				}
				else
				{
					sound->Play();
					playstopButton->SetText("Stop");
				}
			}
			});
		soundWindow->AddWidget(playstopButton);
		playstopButton->SetEnabled(false);

		loopedCheckbox = new asCheckBox("Looped: ");
		loopedCheckbox->SetTooltip("Enable looping for the selected sound instance.");
		loopedCheckbox->SetPos(XMFLOAT2(x + 150, y));
		loopedCheckbox->SetSize(XMFLOAT2(30, 30));
		loopedCheckbox->OnClick([&](asEventArgs args) {
			SoundComponent* sound = GetScene().sounds.GetComponent(entity);
			if (sound != nullptr)
			{
				sound->SetLooped(args.bValue);
			}
			});
		soundWindow->AddWidget(loopedCheckbox);
		loopedCheckbox->SetEnabled(false);

		volumeSlider = new asSlider(0, 1, 1, 1000, "Volume: ");
		volumeSlider->SetTooltip("Set volume level for the selected sound instance.");
		volumeSlider->SetPos(XMFLOAT2(x + 60, y += step));
		volumeSlider->SetSize(XMFLOAT2(240, 30));
		volumeSlider->OnSlide([&](asEventArgs args) {
			SoundComponent* sound = GetScene().sounds.GetComponent(entity);
			if (sound != nullptr)
			{
				sound->volume = args.fValue;
			}
			});
		soundWindow->AddWidget(volumeSlider);
		volumeSlider->SetEnabled(false);

		soundWindow->Translate(XMFLOAT3(400, 120, 0));
		soundWindow->SetVisible(false);

		SetEntity(INVALID_ENTITY);
	}

	SoundWindow::~SoundWindow()
	{
		soundWindow->RemoveWidgets(true);
		GUI->RemoveWidget(soundWindow);
		SAFE_DELETE(soundWindow);
	}



	void SoundWindow::SetEntity(Entity entity)
	{
		this->entity = entity;

		Scene& scene = asScene::GetScene();
		SoundComponent* sound = scene.sounds.GetComponent(entity);
		NameComponent* name = scene.names.GetComponent(entity);

		if (sound != nullptr)
		{
			filenameLabel->SetText(sound->filename);
			if (name == nullptr)
			{
				nameField->SetText("Enter a sound name...");
			}
			else
			{
				nameField->SetText(name->name);
			}
			nameField->SetEnabled(true);
			playstopButton->SetEnabled(true);
			loopedCheckbox->SetEnabled(true);
			loopedCheckbox->SetCheck(sound->IsLooped());
			volumeSlider->SetEnabled(true);
			volumeSlider->SetValue(sound->volume);
			if (sound->IsPlaying())
			{
				playstopButton->SetText("Stop");
			}
			else
			{
				playstopButton->SetText("Play");
			}
		}
		else
		{
			filenameLabel->SetText("");
			nameField->SetText("");
			nameField->SetEnabled(false);
			playstopButton->SetEnabled(false);
			loopedCheckbox->SetEnabled(false);
			volumeSlider->SetEnabled(false);
		}
	}
}

