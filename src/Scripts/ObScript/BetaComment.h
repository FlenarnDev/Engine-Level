#pragma once

namespace Cascadia
{
	namespace ObScript
	{
		class BetaComment
		{
		public:
			static void Install()
			{
				const auto functions = SCRIPT_FUNCTION::GetConsoleFunctions();
				const auto it = std::find_if(
					functions.begin(),
					functions.end(),
					[&](auto&& a_elem)
					{
						return _stricmp(a_elem.functionName, "BetaComment") == 0;
					});

				if (it != functions.end())
				{
					static std::array params{
						SCRIPT_PARAMETER{"String", SCRIPT_PARAM_TYPE::kChar, false},
					};

					*it = SCRIPT_FUNCTION{ LONG_NAME.data(), SHORT_NAME.data(), it->output };
					it->helpString = HelpString().data();
					it->referenceFunction = false;
					it->paramCount = static_cast<std::uint16_t>(params.size());
					it->parameters = params.data();
					it->executeFunction = Execute;

					REX::DEBUG("Registered 'BetaComment' command.");
				}
				else
				{
					REX::WARN("Failed to register 'BetaComment' command.");
				}
			}

		private:
			static bool Execute(
				const SCRIPT_PARAMETER* a_parameters,
				const char* a_compiledParams,
				TESObjectREFR* a_refObject,
				TESObjectREFR* a_container,
				Script* a_script,
				ScriptLocals* a_scriptLocals,
				float&,
				std::uint32_t& a_offset)
			{
				std::array<char, 0x200> rawComment{ '\0' };
				auto result = Script::ParseParameters(
					a_parameters,
					a_compiledParams,
					a_offset,
					a_refObject,
					a_container,
					a_script,
					a_scriptLocals,
					rawComment.data());

				if (!result)
				{
					return true;
				}

				if (rawComment[0] == '\0')
				{
					return true;
				}

				m_refr = a_refObject;
				if (!m_refr)
				{
					ConsoleLog::GetSingleton()->PrintLine("Using Player for BetaComment.");
					m_refr = PlayerCharacter::GetSingleton();
				}

				/*
					%D (%R)							| Current Time
					%s								| File Name
					%D (%R)							| File Time
					%s								| Machine Name
					%08X							| FormID
					%s								| Form Name
					%s								| Cell Name (Int: %s, Ext: %s (%d,%d))
					%08X							| Cell FormID
					%.0f							| Location X
					%.0f							| Location Y
					%.0f							| Location Z
					%.0f							| Angle X
					%.0f							| Angle Y
					%.0f							| Angle Z
					%.0f							| Camera Pos X
					%.0f							| Camera Pos Y
					%.0f							| Camera Pos Z
					%.0f							| Camera Orientation X
					%.0f							| Camera Orientation Y
					%.0f							| Camera Orientation Z
					"%s"							| Comment
				*/

				std::stringstream line;

				PrintCurrentTime(line);
				PrintFileInfo(line);
				PrintMachineName(line);
				PrintFormInfo(line);
				PrintCellInfo(line);
				PrintRefrPositionInfo(line);
				PrintCameraPositionInfo(line);

				REX::DEBUG("");

				// Print Comment
				line << "\"" << rawComment.data() << "\"";
				line << "\r\n";

				_file.open("BetaComments.txt", std::ofstream::out | std::ofstream::app);
				_file << line.str();
				_file.close();

				m_refr = nullptr;
				return true;
			}

			static bool PrintCurrentTime(std::stringstream& a_buf)
			{
				auto currentTime_t = std::chrono::system_clock::now();
				auto currentTime = std::format("{:%m/%d/%y (%H:%M)}"sv, currentTime_t);
				REX::DEBUG("CurrentTime: {:s}", currentTime);

				a_buf << currentTime << _delim;
				return true;
			}

			static bool PrintFileInfo(std::stringstream& a_buf)
			{
				auto file = m_refr->GetFile(0);
				if (file)
				{
					auto fileName = file->GetFilename();
					auto fileTime = std::format("{:%m/%d/%y (%H:%M)}"sv, GetFileTime(file));
					REX::DEBUG("File Name: {:s}", fileName);
					REX::DEBUG("File Time: {:s}", fileTime);

					a_buf << fileName << _delim << fileTime << _delim;
				}
				else
				{
					REX::DEBUG("Warning: No File.");
					a_buf << _delim << _delim;
				}

				return true;
			}

			static bool PrintMachineName(std::stringstream& a_buf)
			{
				const auto machineName = []() -> std::string
					{
						char buffer[16];
						std::uint32_t bufferSize = sizeof(buffer) / sizeof(char);

						// auto result = REX::W32::GetComputerName(buffer, &bufferSize);
						auto result = nullptr;
						return (result) ? buffer : "UNKNOWN"s;
					}();

				REX::DEBUG("Machine Name: {:s}", machineName);

				a_buf << machineName << _delim;
				return true;
			}

			static bool PrintFormInfo(std::stringstream& a_buf)
			{
				auto formID = std::format("{:08X}"sv, m_refr->formID);
				auto formName = m_refr->GetFormEditorID();
				REX::DEBUG("FormID: {:s}", formID);
				REX::DEBUG("Form Name: {:s}", formName);

				a_buf << formID << _delim << formName << _delim;
				return true;
			}

			static bool PrintCellInfo(std::stringstream& a_buf)
			{
				std::stringstream temp;

				auto cell = m_refr->GetParentCell();
				if (cell)
				{
					auto cellID = std::format("{:08X}", cell->formID);

					if (cell->cellFlags.all(TESObjectCELL::Flag::kInterior))
					{
						auto cellName = cell->GetFormEditorID();
						REX::DEBUG("Cell (Interior): {:s}", cellName);
						REX::DEBUG("Cell FormID (Interior): {:s}", cellID);
						temp << cellName << _delim << cellID << _delim;
					}
					else
					{
						if ((cell->formFlags >> 13) & 1)
						{
							REX::WARN("BetaComment::PrintCellInfo: Unknown Edge Case.");
							temp << _delim << _delim;
						}
						else
						{
							auto cellX = cell->GetDataX();
							auto cellY = cell->GetDataY();
							auto cellName = std::format(
								"{:s} ({:d},{:d})",
								cell->worldSpace->GetFormEditorID(),
								cellX,
								cellY);
							REX::DEBUG("Cell (Exterior): {:s}", cellName);
							REX::DEBUG("Cell FormID (Exterior): {:s}", cellID);
							temp << cellName << _delim << cellID << _delim;
						}
					}
				}
				else
				{
					REX::DEBUG("Cell Info: No Cell.");
					temp << _delim << _delim;
				}

				a_buf << temp.str();
				return true;
			}

			static bool PrintRefrPositionInfo(std::stringstream& a_buf)
			{
				auto locationX = std::format("{:.0f}"sv, m_refr->data.location.x);
				auto locationY = std::format("{:.0f}"sv, m_refr->data.location.y);
				auto locationZ = std::format("{:.0f}"sv, m_refr->data.location.z);
				REX::DEBUG("Refr Location: {:s}, {:s}, {:s}", locationX, locationY, locationZ);

				auto angleX = std::format("{:.0f}"sv, m_refr->data.angle.x);
				auto angleY = std::format("{:.0f}"sv, m_refr->data.angle.y);
				auto angleZ = std::format("{:.0f}"sv, m_refr->data.angle.z);
				REX::DEBUG("Refr Angle: {:s}, {:s}, {:s}", angleX, angleY, angleZ);

				a_buf
					<< locationX << _delim
					<< locationY << _delim
					<< locationZ << _delim
					<< angleX << _delim
					<< angleY << _delim
					<< angleZ << _delim;
				return true;
			}

			static bool PrintCameraPositionInfo(std::stringstream& a_buf)
			{
				auto rootCamera = Main::WorldRootCamera();
				auto cameraPositionX = std::format("{:.0f}"sv, rootCamera->world.translate.x);
				auto cameraPositionY = std::format("{:.0f}"sv, rootCamera->world.translate.y);
				auto cameraPositionZ = std::format("{:.0f}"sv, rootCamera->world.translate.z);
				REX::DEBUG("Camera Position: {:s}, {:s}, {:s}", cameraPositionX, cameraPositionY, cameraPositionZ);

				float fCameraAngleX{ 0.0f }, fCameraAngleY{ 0.0f }, fCameraAngleZ{ 0.0f };
				rootCamera->parent->world.rotate.ToEulerAnglesXYZ(fCameraAngleX, fCameraAngleY, fCameraAngleZ);
				auto cameraAngleX = std::format("{:.0f}"sv, fCameraAngleX);
				auto cameraAngleY = std::format("{:.0f}"sv, fCameraAngleY);
				auto cameraAngleZ = std::format("{:.0f}"sv, fCameraAngleZ);
				REX::DEBUG("Camera Angle: {:s}, {:s}, {:s}", cameraAngleX, cameraAngleY, cameraAngleZ);

				a_buf
					<< cameraPositionX << _delim
					<< cameraPositionY << _delim
					<< cameraPositionZ << _delim
					<< cameraAngleX << _delim
					<< cameraAngleY << _delim
					<< cameraAngleZ << _delim;
				return true;
			}

			[[nodiscard]] static const std::string& HelpString()
			{
				static auto help = []()
					{
						std::string buf;
						buf += "Add comment to "sv;
						buf += "BetaComments.txt";
						buf += ". [bc \"This is clipping.\"]"sv;
						return buf;
					}();
				return help;
			}

			[[nodiscard]] static std::chrono::system_clock::time_point GetFileTime(TESFile* a_file)
			{
				union
				{
					struct
					{
						std::uint32_t LowPart;
						std::uint32_t HighPart;
					} s;

					std::uint64_t QuadPart;
				} ull;

				ull.s.LowPart = a_file->fileInfo.modifyTime.lo;
				ull.s.HighPart = a_file->fileInfo.modifyTime.hi;

				auto time_t = static_cast<std::time_t>(ull.QuadPart / 10000000ULL - 11644473600ULL);
				return std::chrono::system_clock::from_time_t(time_t);
			}

			static constexpr auto LONG_NAME = "BetaComment"sv;
			static constexpr auto SHORT_NAME = "bc"sv;

			inline static TESObjectREFR* m_refr{ nullptr };
			inline static char _delim{ '\t' };

			inline static std::ofstream _file;
		};
	}
}