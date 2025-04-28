#include "pch.h"
#include "Curve.h"
#include "FileUtils.h"

void Curve::ShowJumpList()
{
	ImGuiChildFlags flags = ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY;
	if (ImGui::BeginChild("cc tabs", ImVec2(400.0f, -1), flags))
	{
		ImGui::Text("Cooling force curves");
		if (ImGui::BeginListBox("##jump list", ImVec2(-1, 500)))
		{
			int i = 0;
			for (const PhaseJump& jump : jumps)
			{
				ImGui::PushID(i++);
				if (jump.ShowAsListItem(selectedIndex == i))
				{
					selectedIndex = i;
					SelectedItemChanged();
				}
				ImGui::PopID();
			}

			ImGui::EndListBox();
		}
	}
	ImGui::EndChild();
}

void Curve::LoadPhaseJumpFolder(std::filesystem::path inputFolder)
{
	if (std::filesystem::is_directory(inputFolder))
	{
		for (const auto& entry : std::filesystem::directory_iterator(inputFolder))
		{
			if (entry.is_regular_file() && (entry.path().extension() == ".CSV" || entry.path().extension() == ".csv"))
			{
				std::filesystem::path file = entry.path();
				PhaseJump jump;
				jump.LoadFromFile(file);

				AddPhaseJump(std::move(jump));
			}
		}
		folder = inputFolder.parent_path().parent_path().filename() / inputFolder.parent_path().filename();
	}
}
