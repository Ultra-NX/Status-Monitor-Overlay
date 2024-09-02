class MicroOverlay : public tsl::Gui {
private:
	uint64_t mappedButtons = MapButtons(keyCombo); // map buttons
	char GPU_Load_c[32] = "";
	char Rotation_SpeedLevel_c[64] = "";
	char RAM_var_compressed_c[128] = "";
	char Power_c[64] = "";
	char CPU_compressed_c[160] = "";
	char CPUS_compressed_c[160] = "";
	char CPU_Usage[32] = "";
	char CPU_Usage0[32] = "";
	char CPU_Usage1[32] = "";
	char CPU_Usage2[32] = "";
	char CPU_Usage3[32] = "";
	char skin_temperature_c[48] = "";
	char skin_temperatureB_c[48] = "";
	char skin_temperatureS_c[48] = "";
	char batteryCharge[10] = ""; // Declare the batteryCharge variable
	char FPS_var_compressed_c[64] = "";
	char Battery_c[32];
	char BatteryS_c[32];
	char CPU_volt_c[10];
	char GPU_volt_c[10];
	char RAM_volt_c[16];
	char SOC_volt_c[10];

	uint32_t margin = 8;

	std::pair<u32, u32> CPU_dimensions;
	std::pair<u32, u32> CPUB_dimensions;
	std::pair<u32, u32> GPU_dimensions;
	std::pair<u32, u32> RAM_dimensions;
	std::pair<u32, u32> BRD_dimensions;
	std::pair<u32, u32> BRDB_dimensions;
	std::pair<u32, u32> BRDM_dimensions;
	std::pair<u32, u32> FAN_dimensions;
	std::pair<u32, u32> PWR_dimensions;
	std::pair<u32, u32> BAT_dimensions;
	std::pair<u32, u32> BATB_dimensions;
	std::pair<u32, u32> FPS_dimensions;
	bool Initialized = false;
	MicroSettings settings;
	size_t text_width = 0;
	size_t fps_width = 0;
	size_t bat_width = 0;
	ApmPerformanceMode performanceMode = ApmPerformanceMode_Invalid;
	size_t fontsize = 0;
	bool showFPS = false;
	uint64_t systemtickfrequency_impl = systemtickfrequency;
public:
    MicroOverlay() { 
		GetConfigSettings(&settings);
		apmGetPerformanceMode(&performanceMode);
		if (performanceMode == ApmPerformanceMode_Normal) {
			fontsize = settings.handheldFontSize;
		}
		else fontsize = settings.dockedFontSize;
		if (settings.setPosBottom) {
			tsl::gfx::Renderer::getRenderer().setLayerPos(0, 1038);
		}
		mutexInit(&mutex_BatteryChecker);
		mutexInit(&mutex_Misc);
		TeslaFPS = settings.refreshRate;
		systemtickfrequency_impl /= settings.refreshRate;
		alphabackground = 0x0;
		deactivateOriginalFooter = true;
        StartThreads();
	}
	~MicroOverlay() {
		CloseThreads();
	}
    
    virtual tsl::elm::Element* createUI() override {
		rootFrame = new tsl::elm::OverlayFrame("", "");

		auto Status = new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, u16 x, u16 y, u16 w, u16 h) {

			if (!Initialized) {
				CPU_dimensions = renderer->drawString("CPU 100%△1444.4", false, 0, fontsize, fontsize, renderer->a(0x0000));
				CPUB_dimensions = renderer->drawString("CPU 100%,100%,100%,100%△1444.4", false, 0, fontsize, fontsize, renderer->a(0x0000));
				GPU_dimensions = renderer->drawString("GPU 99.9%△1444.4", false, 0, fontsize, fontsize, renderer->a(0x0000));
				if (R_FAILED(sysclkCheck) || !settings.showRAMLoad) {
					RAM_dimensions = renderer->drawString("RAM 4.4/44.4GB△4444.4", false, 0, fontsize, fontsize, renderer->a(0x0000));
				}
				else RAM_dimensions = renderer->drawString("RAM 100.0%△4444.4", false, 0, fontsize, fontsize, renderer->a(0x0000));
				BRD_dimensions = renderer->drawString("BRD 88.8\u00B0C(100%)", false, 0, fontsize, fontsize, renderer->a(0x0000));
				BRDM_dimensions = renderer->drawString("BRD 88.8/88.8/88.8\u00B0C@-15.5W[19:99]", false, 0, fontsize, fontsize, renderer->a(0x0000));
				BRDB_dimensions = renderer->drawString("BRD 88.8/88.8/88.8\u00B0C(100%)", false, 0, fontsize, fontsize, renderer->a(0x0000));
				FAN_dimensions = renderer->drawString("FAN 100.0% ", false, 0, fontsize, fontsize, renderer->a(0x0000));
				PWR_dimensions = renderer->drawString("PWR -15.5W", false, 0, fontsize, fontsize, renderer->a(0x0000));
				FPS_dimensions = renderer->drawString("FPS 444.4", false, 0, fontsize, fontsize, renderer->a(0x0000));
				BAT_dimensions = renderer->drawString("100.0%[44:44]", false, 0, fontsize, fontsize, renderer->a(0x0000));
				BATB_dimensions = renderer->drawString("BAT 99.9%(-15.5W)[9:99]", false, 0, fontsize, fontsize, renderer->a(0x0000));
				auto spacesize = renderer->drawString(" ", false, 0, fontsize, fontsize, renderer->a(0x0000));
				margin = spacesize.first;
				text_width = 0;
				int8_t entry_count = -1;
				uint8_t flags = 0;
				for (std::string key : tsl::hlp::split(settings.show, '+')) {
					if (!key.compare("CPUB") && !(flags & 1 << 0)) {
						text_width += CPUB_dimensions.first;
						entry_count += 1;
						flags |= 1 << 0;
					}
					else if (!key.compare("CPU") && !(flags & 1 << 1)) {
						text_width += CPU_dimensions.first;
						entry_count += 1;
						flags |= 1 << 1;
					}
					else if (!key.compare("GPU") && !(flags & 1 << 2)) {
						text_width += GPU_dimensions.first;
						entry_count += 1;
						flags |= 1 << 2;
					}
					else if (!key.compare("RAM") && !(flags & 1 << 3)) {
						text_width += RAM_dimensions.first;
						entry_count += 1;
						flags |= 1 << 3;
					}
					else if (!key.compare("BRD") && !(flags & 1 << 4)) {
						text_width += BRD_dimensions.first;
						entry_count += 1;
						flags |= 1 << 4;
					}
					else if (!key.compare("BRDM") && !(flags & 1 << 5)) {
						text_width += BRDM_dimensions.first;
						entry_count += 1;
						flags |= 1 << 5;
					}
					else if (!key.compare("BRDB") && !(flags & 1 << 6)) {
						text_width += BRDB_dimensions.first;
						entry_count += 1;
						flags |= 1 << 6;
					}
					else if (!key.compare("PWR") && !(flags & 1 << 7)) {
						text_width += PWR_dimensions.first;
						entry_count += 1;
						flags |= 1 << 7;
					}
					else if (!key.compare("BATB") && !(flags & 1 << 8)) {
						text_width += BATB_dimensions.first;
						entry_count += 1;
						flags |= 1 << 8;
					}
					else if (!key.compare("FAN") && !(flags & 1 << 9)) {
						text_width += FAN_dimensions.first;
						entry_count += 1;
						flags |= 1 << 9;
					}
					else if (!key.compare("FPS") && !(flags & 1 << 10)) {
						fps_width = FPS_dimensions.first;
						showFPS = true;
						flags |= 1 << 10;
					}
					else if (!key.compare("BAT") && !(flags & 1 << 11)) {
						bat_width = BAT_dimensions.first;
						flags |= 1 << 11;
					}
					else if (!key.compare("FPSE") && !(flags & 1 << 12)) {
						fps_width = FPS_dimensions.first;
						showFPS = true;
						flags |= 1 << 12;
					}
					else if (!key.compare("BATE") && !(flags & 1 << 13)) {
						bat_width = BATB_dimensions.first;
						flags |= 1 << 13;
					}
				}
				text_width += (margin * entry_count);
				Initialized = true;
				tsl::hlp::requestForeground(false);
			}

			u32 base_y = 0;
			if (settings.setPosBottom) {
				base_y = tsl::cfg::FramebufferHeight - (fontsize + (fontsize / 4));
			}

			renderer->drawRect(0, base_y, tsl::cfg::FramebufferWidth, fontsize + (fontsize / 4), a(settings.backgroundColor));

			uint32_t offset = 0;
			if (settings.alignTo == 1) {
				if (GameRunning && showFPS) {
					offset = (tsl::cfg::FramebufferWidth - (text_width + fps_width)) / 2;
				}
				else offset = (tsl::cfg::FramebufferWidth - text_width) / 2;
			}
			else if (settings.alignTo == 2) {
				if (GameRunning && showFPS) {
					offset = tsl::cfg::FramebufferWidth - (text_width + fps_width);
				}
				else offset = tsl::cfg::FramebufferWidth - text_width;
			}
			uint8_t flags = 0;
			for (std::string key : tsl::hlp::split(settings.show, '+')) {
				if (!key.compare("CPUB") && !(flags & 1 << 0)) {
					auto dimensions_s = renderer->drawString("CPU", false, offset, base_y+fontsize, fontsize, renderer->a(settings.catColor));
					uint32_t offset_s = offset + dimensions_s.first + margin;
					renderer->drawString(CPU_compressed_c, false, offset_s, base_y+fontsize, fontsize, renderer->a(settings.textColor));
					offset += CPUB_dimensions.first + margin;
					if (settings.realVolts) {
						auto dimensions_v = renderer->drawString(" | 1444mV", false, 0, fontsize, fontsize, renderer->a(0x0000));
						offset += dimensions_v.first + margin;
					}
					flags |= 1 << 0;
				}
				else if (!key.compare("CPU") && !(flags & 1 << 1)) {
					auto dimensions_s = renderer->drawString("CPU", false, offset, base_y+fontsize, fontsize, renderer->a(settings.catColor));
					uint32_t offset_s = offset + dimensions_s.first + margin;
					renderer->drawString(CPUS_compressed_c, false, offset_s, base_y+fontsize, fontsize, renderer->a(settings.textColor));
					offset += CPU_dimensions.first + margin;
					if (settings.realVolts) {
						auto dimensions_v = renderer->drawString(" | 1444mV", false, 0, fontsize, fontsize, renderer->a(0x0000));
						offset += dimensions_v.first + margin;
					}
					flags |= 1 << 1;
				}
				else if (!key.compare("GPU") && !(flags & 1 << 2)) {
					auto dimensions_s = renderer->drawString("GPU", false, offset, base_y+fontsize, fontsize, renderer->a(settings.catColor));
					uint32_t offset_s = offset + dimensions_s.first + margin;
					renderer->drawString(GPU_Load_c, false, offset_s, base_y+fontsize, fontsize, renderer->a(settings.textColor));
					offset += GPU_dimensions.first + margin;
					if (settings.realVolts) {
						auto dimensions_v = renderer->drawString(" | 1444mV", false, 0, fontsize, fontsize, renderer->a(0x0000));
						offset += dimensions_v.first + margin;
					}
					flags |= 1 << 2;
				}
				else if (!key.compare("RAM") && !(flags & 1 << 3)) {
					auto dimensions_s = renderer->drawString("RAM", false, offset, base_y+fontsize, fontsize, renderer->a(settings.catColor));
					uint32_t offset_s = offset + dimensions_s.first + margin;
					renderer->drawString(RAM_var_compressed_c, false, offset_s, base_y+fontsize, fontsize, renderer->a(settings.textColor));
					offset += RAM_dimensions.first + margin;
					if (settings.realVolts) {
						auto dimensions_v = renderer->drawString(" | 1444/1444mV", false, 0, fontsize, fontsize, renderer->a(0x0000));
						offset += dimensions_v.first + margin;
					}
					flags |= 1 << 3;
				}
				else if (!key.compare("BRD") && !(flags & 1 << 4)) {
					auto dimensions_s = renderer->drawString("BRD", false, offset, base_y+fontsize, fontsize, renderer->a(settings.catColor));
					uint32_t offset_s = offset + dimensions_s.first + margin;
					renderer->drawString(skin_temperatureS_c, false, offset_s, base_y+fontsize, fontsize, renderer->a(settings.textColor));
					offset += BRD_dimensions.first + margin;
					if (settings.realVolts) {
						auto dimensions_v = renderer->drawString(" | 1444mV", false, 0, fontsize, fontsize, renderer->a(0x0000));
						offset += dimensions_v.first + margin;
					}
					flags |= 1 << 4;
				}
				else if (!key.compare("BRDM") && !(flags & 1 << 5)) {
					auto dimensions_s = renderer->drawString("BRD", false, offset, base_y+fontsize, fontsize, renderer->a(settings.catColor));
					uint32_t offset_s = offset + dimensions_s.first + margin;
					renderer->drawString(skin_temperature_c, false, offset_s, base_y+fontsize, fontsize, renderer->a(settings.textColor));
					offset += BRDM_dimensions.first + margin;
					if (settings.realVolts) {
						auto dimensions_v = renderer->drawString(" | 1444mV", false, 0, fontsize, fontsize, renderer->a(0x0000));
						offset += dimensions_v.first + margin;
					}
					flags |= 1 << 5;
				}
				else if (!key.compare("BRDB") && !(flags & 1 << 6)) {
					auto dimensions_s = renderer->drawString("BRD", false, offset, base_y+fontsize, fontsize, renderer->a(settings.catColor));
					uint32_t offset_s = offset + dimensions_s.first + margin;
					renderer->drawString(skin_temperatureB_c, false, offset_s, base_y+fontsize, fontsize, renderer->a(settings.textColor));
					offset += BRDB_dimensions.first + margin + 10;
					if (settings.realVolts) {
						auto dimensions_v = renderer->drawString(" | 1444mV", false, 0, fontsize, fontsize, renderer->a(0x0000));
						offset += dimensions_v.first + margin;
					}
					flags |= 1 << 6;
				}
				else if (!key.compare("PWR") && !(flags & 1 << 7)) {
					auto dimensions_s = renderer->drawString("PWR", false, offset, base_y+fontsize, fontsize, renderer->a(settings.catColor));
					uint32_t offset_s = offset + dimensions_s.first + margin;
					renderer->drawString(Power_c, false, offset_s, base_y+fontsize, fontsize, renderer->a(settings.textColor));
					offset += PWR_dimensions.first + margin;
					flags |= 1 << 7;
				}
				else if (!key.compare("BATB") && !(flags & 1 << 8)) {
					auto dimensions_s = renderer->drawString("BAT", false, offset, base_y+fontsize, fontsize, renderer->a(settings.catColor));
					uint32_t offset_s = offset + dimensions_s.first + margin;
					renderer->drawString(Battery_c, false, offset_s, base_y+fontsize, fontsize, renderer->a(settings.textColor));
					offset += BATB_dimensions.first + margin;
					flags |= 1 << 8;
				}
				else if (!key.compare("FAN") && !(flags & 1 << 9)) {
					auto dimensions_s = renderer->drawString("FAN", false, offset, base_y+fontsize, fontsize, renderer->a(settings.catColor));
					uint32_t offset_s = offset + dimensions_s.first + margin;
					renderer->drawString(Rotation_SpeedLevel_c, false, offset_s, base_y+fontsize, fontsize, renderer->a(settings.textColor));
					offset += FAN_dimensions.first + margin;
					flags |= 1 << 9;
				}
				else if (!key.compare("FPS") && GameRunning && !(flags & 1 << 10)) {
					auto dimensions_s = renderer->drawString("FPS", false, offset, base_y+fontsize, fontsize, renderer->a(settings.catColor));
					uint32_t offset_s = offset + dimensions_s.first + margin;
					renderer->drawString(FPS_var_compressed_c, false, offset_s, base_y+fontsize, fontsize, renderer->a(settings.textColor));
					offset += FPS_dimensions.first + margin;
					flags |= 1 << 10;
				}
				else if (!key.compare("BAT") && !(flags & 1 << 11)) {
					renderer->drawString(BatteryS_c, false, tsl::cfg::FramebufferWidth - BAT_dimensions.first, base_y+fontsize, fontsize, renderer->a(settings.textColor));
					offset += PWR_dimensions.first + margin;
					flags |= 1 << 11;
				}
				else if (!key.compare("FPSE") && GameRunning && !(flags & 1 << 12)) {
					offset = tsl::cfg::FramebufferWidth - FPS_dimensions.first - 5;
					auto dimensions_s = renderer->drawString("FPS", false, offset, base_y+fontsize, fontsize, renderer->a(settings.catColor));
					uint32_t offset_s = offset + dimensions_s.first + margin;
					renderer->drawString(FPS_var_compressed_c, false, offset_s, base_y+fontsize, fontsize, renderer->a(settings.textColor));
					flags |= 1 << 12;
				}
				else if (!key.compare("BATE") && !(flags & 1 << 13)) {
					offset = tsl::cfg::FramebufferWidth - BATB_dimensions.first;
					auto dimensions_s = renderer->drawString("BAT", false, offset, base_y+fontsize, fontsize, renderer->a(settings.catColor));
					uint32_t offset_s = offset + dimensions_s.first + margin;
					renderer->drawString(Battery_c, false, offset_s, base_y+fontsize, fontsize, renderer->a(settings.textColor));
					flags |= 1 << 13;
				}
			}
		});

		rootFrame->setContent(Status);

		return rootFrame;
	}

	virtual void update() override {
		apmGetPerformanceMode(&performanceMode);
		if (performanceMode == ApmPerformanceMode_Normal) {
			if (fontsize != settings.handheldFontSize) {
				Initialized = false;
				fontsize = settings.handheldFontSize;
			}
		}
		else if (performanceMode == ApmPerformanceMode_Boost) {
			if (fontsize != settings.dockedFontSize) {
				Initialized = false;
				fontsize = settings.dockedFontSize;
			}
		}

		//Make stuff ready to print
		///CPU
		/* if (idletick0 > systemtickfrequency_impl)
			strcpy(CPU_Usage0, "0%");
		else snprintf(CPU_Usage0, sizeof CPU_Usage0, "%.0f%%", (1.d - ((double)idletick0 / systemtickfrequency_impl)) * 100);
		if (idletick1 > systemtickfrequency_impl)
			strcpy(CPU_Usage1, "0%");
		else snprintf(CPU_Usage1, sizeof CPU_Usage1, "%.0f%%", (1.d - ((double)idletick1 / systemtickfrequency_impl)) * 100);
		if (idletick2 > systemtickfrequency_impl)
			strcpy(CPU_Usage2, "0%");
		else snprintf(CPU_Usage2, sizeof CPU_Usage2, "%.0f%%", (1.d - ((double)idletick2 / systemtickfrequency_impl)) * 100);
		if (idletick3 > systemtickfrequency_impl)
			strcpy(CPU_Usage3, "0%");
		else snprintf(CPU_Usage3, sizeof CPU_Usage3, "%.0f%%", (1.d - ((double)idletick3 / systemtickfrequency_impl)) * 100); */

		if (idletick0 > systemtickfrequency_impl) idletick0 = systemtickfrequency_impl;
		if (idletick1 > systemtickfrequency_impl) idletick1 = systemtickfrequency_impl;
		if (idletick2 > systemtickfrequency_impl) idletick2 = systemtickfrequency_impl;
		if (idletick3 > systemtickfrequency_impl) idletick3 = systemtickfrequency_impl;
		double cpu_usage0 = (1.d - ((double)idletick0 / systemtickfrequency_impl)) * 100;
		double cpu_usage1 = (1.d - ((double)idletick1 / systemtickfrequency_impl)) * 100;
		double cpu_usage2 = (1.d - ((double)idletick2 / systemtickfrequency_impl)) * 100;
		double cpu_usage3 = (1.d - ((double)idletick3 / systemtickfrequency_impl)) * 100;
		double cpu_usageM = 0;
		if (cpu_usage0 > cpu_usageM)	cpu_usageM = cpu_usage0;
		if (cpu_usage1 > cpu_usageM)	cpu_usageM = cpu_usage1;
		if (cpu_usage2 > cpu_usageM)	cpu_usageM = cpu_usage2;
		if (cpu_usage3 > cpu_usageM)	cpu_usageM = cpu_usage3;
		
		//Make stuff ready to print
		///CPU
		snprintf(CPU_Usage, sizeof CPU_Usage0, "%.0f%%", cpu_usageM);
		snprintf(CPU_Usage0, sizeof CPU_Usage0, "%.0f%%", (1.d - ((double)idletick0 / systemtickfrequency)) * 100);
		snprintf(CPU_Usage1, sizeof CPU_Usage1, "%.0f%%", (1.d - ((double)idletick1 / systemtickfrequency)) * 100);
		snprintf(CPU_Usage2, sizeof CPU_Usage2, "%.0f%%", (1.d - ((double)idletick2 / systemtickfrequency)) * 100);
		snprintf(CPU_Usage3, sizeof CPU_Usage3, "%.0f%%", (1.d - ((double)idletick3 / systemtickfrequency)) * 100);

		mutexLock(&mutex_Misc);
		char difference[5] = "@";
		if (realCPU_Hz) {
			int32_t deltaCPU = (int32_t)(realCPU_Hz / 1000) - (CPU_Hz / 1000);
			if (deltaCPU > 20000) {
				strcpy(difference, "△");
			}
			else if (deltaCPU < -50000) {
				strcpy(difference, "≠");
			}
			else if (deltaCPU < -20000) {
				strcpy(difference, "▽");
			}
		}
		if (settings.realVolts) {
			if (settings.realFrequencies && realCPU_Hz) {
				snprintf(CPU_compressed_c, sizeof CPU_compressed_c, 
					"%s,%s,%s,%s%s%d.%d | %dmV", 
					CPU_Usage0, CPU_Usage1, CPU_Usage2, CPU_Usage3, 
					difference, 
					realCPU_Hz / 1000000, (realCPU_Hz / 100000) % 10,
					realCPU_mV);

				snprintf(CPUS_compressed_c, sizeof CPUS_compressed_c, 
					"%s%s%d.%d | %dmV", 
					CPU_Usage, 
					difference, 
					realCPU_Hz / 1000000, (realCPU_Hz / 100000) % 10,
					realCPU_mV);
			}
			else {
				snprintf(CPU_compressed_c, sizeof CPU_compressed_c, 
					"%s,%s,%s,%s%s%d.%d | %dmV", 
					CPU_Usage0, CPU_Usage1, CPU_Usage2, CPU_Usage3, 
					difference, 
					CPU_Hz / 1000000, (CPU_Hz / 100000) % 10,
					realCPU_mV);

				snprintf(CPUS_compressed_c, sizeof CPUS_compressed_c, 
					"%s%s%d.%d | %dmV", 
					CPU_Usage,
					difference, 
					CPU_Hz / 1000000, (CPU_Hz / 100000) % 10,
					realCPU_mV);
			}
		}
		else {	
			if (settings.realFrequencies && realCPU_Hz) {
				snprintf(CPU_compressed_c, sizeof CPU_compressed_c, 
					"%s,%s,%s,%s%s%d.%d", 
					CPU_Usage0, CPU_Usage1, CPU_Usage2, CPU_Usage3, 
					difference, 
					realCPU_Hz / 1000000, (realCPU_Hz / 100000) % 10);

				snprintf(CPUS_compressed_c, sizeof CPUS_compressed_c, 
					"%s%s%d.%d", 
					CPU_Usage, 
					difference, 
					realCPU_Hz / 1000000, (realCPU_Hz / 100000) % 10);
			}
			else {
				snprintf(CPU_compressed_c, sizeof CPU_compressed_c, 
					"%s,%s,%s,%s%s%d.%d", 
					CPU_Usage0, CPU_Usage1, CPU_Usage2, CPU_Usage3, 
					difference, 
					CPU_Hz / 1000000, (CPU_Hz / 100000) % 10);

				snprintf(CPUS_compressed_c, sizeof CPUS_compressed_c, 
					"%s%s%d.%d", 
					CPU_Usage,
					difference, 
					CPU_Hz / 1000000, (CPU_Hz / 100000) % 10);
			}
		}
		
		///GPU
		if (realGPU_Hz) {
			int32_t deltaGPU = (int32_t)(realGPU_Hz / 1000) - (GPU_Hz / 1000);
			if (deltaGPU >= 20000) {
				strcpy(difference, "△");
			}
			else if (deltaGPU > -20000) {
				strcpy(difference, "@");
			}
			else if (deltaGPU < -50000) {
				strcpy(difference, "≠");
			}
			else if (deltaGPU < -20000) {
				strcpy(difference, "▽");
			}
		}
		else {
			strcpy(difference, "@");
		}
		if (settings.realFrequencies && realGPU_Hz) {
			snprintf(GPU_Load_c, sizeof GPU_Load_c, 
				"%d.%d%%%s%d.%d", 
				GPU_Load_u / 10, GPU_Load_u % 10, 
				difference, 
				realGPU_Hz / 1000000, (realGPU_Hz / 100000) % 10);
			if (settings.realVolts) {
				snprintf(GPU_volt_c, sizeof GPU_volt_c, " | %dmV", realGPU_mV);
				strncat(GPU_Load_c, GPU_volt_c, sizeof GPU_Load_c);
			}
		}
		else {
			snprintf(GPU_Load_c, sizeof GPU_Load_c, 
				"%d.%d%%%s%d.%d", 
				GPU_Load_u / 10, GPU_Load_u % 10, 
				difference,
				GPU_Hz / 1000000, (GPU_Hz / 100000) % 10);
			if (settings.realVolts) {
				snprintf(GPU_volt_c, sizeof GPU_volt_c, " | %dmV", realGPU_mV);
				strncat(GPU_Load_c, GPU_volt_c, sizeof GPU_Load_c);
			}
		}
		
		///RAM
		char MICRO_RAM_all_c[12] = "";
		if (!settings.showRAMLoad || R_FAILED(sysclkCheck)) {
			float RAM_Total_application_f = (float)RAM_Total_application_u / 1024 / 1024;
			float RAM_Total_applet_f = (float)RAM_Total_applet_u / 1024 / 1024;
			float RAM_Total_system_f = (float)RAM_Total_system_u / 1024 / 1024;
			float RAM_Total_systemunsafe_f = (float)RAM_Total_systemunsafe_u / 1024 / 1024;
			float RAM_Total_all_f = RAM_Total_application_f + RAM_Total_applet_f + RAM_Total_system_f + RAM_Total_systemunsafe_f;
			float RAM_Used_application_f = (float)RAM_Used_application_u / 1024 / 1024;
			float RAM_Used_applet_f = (float)RAM_Used_applet_u / 1024 / 1024;
			float RAM_Used_system_f = (float)RAM_Used_system_u / 1024 / 1024;
			float RAM_Used_systemunsafe_f = (float)RAM_Used_systemunsafe_u / 1024 / 1024;
			float RAM_Used_all_f = RAM_Used_application_f + RAM_Used_applet_f + RAM_Used_system_f + RAM_Used_systemunsafe_f;
			snprintf(MICRO_RAM_all_c, sizeof(MICRO_RAM_all_c), "%.1f/%.1fGB", RAM_Used_all_f/1024, RAM_Total_all_f/1024);
		}
		else {
			snprintf(MICRO_RAM_all_c, sizeof(MICRO_RAM_all_c), "%hu.%hhu%%", ramLoad[SysClkRamLoad_All] / 10, ramLoad[SysClkRamLoad_All] % 10);
		}

		if (realRAM_Hz) {
			int32_t deltaRAM = (int32_t)(realRAM_Hz / 1000) - (RAM_Hz / 1000);
			if (deltaRAM >= 20000) {
				strcpy(difference, "△");
			}
			else if (deltaRAM > -20000) {
				strcpy(difference, "@");
			}
			else if (deltaRAM < -50000) {
				strcpy(difference, "≠");
			}
			else if (deltaRAM < -20000) {
				strcpy(difference, "▽");
			}
		}
		else {
			strcpy(difference, "@");
		}
		if (settings.realFrequencies && realRAM_Hz) {
			snprintf(RAM_var_compressed_c, sizeof RAM_var_compressed_c, 
				"%s%s%d.%d", 
				MICRO_RAM_all_c, difference, realRAM_Hz / 1000000, (realRAM_Hz / 100000) % 10);
			if (settings.realVolts) {
				snprintf(RAM_volt_c, sizeof RAM_volt_c, " | %d/%dmV", realRAM_mV/10000, realRAM_mV%10000);
				strncat(RAM_var_compressed_c, RAM_volt_c, sizeof RAM_var_compressed_c);
			}
		}
		else {
			 snprintf(RAM_var_compressed_c, sizeof RAM_var_compressed_c, 
				"%s%s%d.%d", 
				MICRO_RAM_all_c, difference, RAM_Hz / 1000000, (RAM_Hz / 1000000) % 10);
			if (settings.realVolts) {
				snprintf(RAM_volt_c, sizeof RAM_volt_c, " | %d/%dmV", realRAM_mV/10000, realRAM_mV%10000);
				strncat(RAM_var_compressed_c, RAM_volt_c, sizeof RAM_var_compressed_c);
			}
		}
		
		///Battery
		char remainingBatteryLife[8];
		mutexLock(&mutex_BatteryChecker);
		if (batTimeEstimate >= 0) {
			snprintf(remainingBatteryLife, sizeof remainingBatteryLife, "%d:%02d", batTimeEstimate / 60, batTimeEstimate % 60);
		}
		else snprintf(remainingBatteryLife, sizeof remainingBatteryLife, "--:--");
		snprintf(BatteryS_c, sizeof Battery_c, "%.1f%s [%s]", (float)_batteryChargeInfoFields.RawBatteryCharge / 1000, "%", remainingBatteryLife);

		snprintf(Battery_c, sizeof Battery_c, "%.1f%s(%+.1fW)[%s]", (float)_batteryChargeInfoFields.RawBatteryCharge / 1000, "%", PowerConsumption, remainingBatteryLife);

		snprintf(Power_c, sizeof Power_c, "%0.2fW", PowerConsumption);

		///Thermal
		if (settings.realVolts) {
			if (hosversionAtLeast(10,0,0)) {
				snprintf(skin_temperature_c, sizeof skin_temperature_c, 
					"%2.1f/%2.1f/%hu.%hhu\u00B0C@%+.1fW[%s] | %dmV", 
					SOC_temperatureF, PCB_temperatureF, 
					skin_temperaturemiliC / 1000, (skin_temperaturemiliC / 100) % 10, 
					PowerConsumption, remainingBatteryLife,
					realSOC_mV);

				snprintf(skin_temperatureB_c, sizeof skin_temperatureB_c, 
					"%2.1f/%2.1f/%hu.%hhu\u00B0C(%2.1f%%) | %dmV", 
					SOC_temperatureF, PCB_temperatureF, 
					skin_temperaturemiliC / 1000, (skin_temperaturemiliC / 100) % 10,
					Rotation_SpeedLevel_f * 100,
					realSOC_mV);

				snprintf(skin_temperatureS_c, sizeof skin_temperatureS_c, 
					"%2.1f\u00B0C(%2.0f%%) | %dmV", 
					SOC_temperatureF,
					Rotation_SpeedLevel_f * 100,
					realSOC_mV);
			}
			else {
				snprintf(skin_temperature_c, sizeof skin_temperature_c, 
					"%hu.%hhu/%hu.%hhu/%hu.%hhu\u00B0C@%+.1fW[%s] | %dmV", 
					SOC_temperatureC / 1000, (SOC_temperatureC / 100) % 10, 
					PCB_temperatureC / 1000, (PCB_temperatureC / 100) % 10, 
					skin_temperaturemiliC / 1000, (skin_temperaturemiliC / 100) % 10, 
					PowerConsumption, remainingBatteryLife,
					realSOC_mV);
					
				snprintf(skin_temperatureB_c, sizeof skin_temperatureB_c, 
					"%hu.%hhu/%hu.%hhu/%hu.%hhu\u00B0C(%2.1f%%) | %dmV", 
					SOC_temperatureC / 1000, (SOC_temperatureC / 100) % 10, 
					PCB_temperatureC / 1000, (PCB_temperatureC / 100) % 10, 
					skin_temperaturemiliC / 1000, (skin_temperaturemiliC / 100) % 10,
					Rotation_SpeedLevel_f * 100,
					realSOC_mV);

				snprintf(skin_temperatureS_c, sizeof skin_temperatureS_c, 
					"%hu.%hhu\u00B0C(%2.0f%%) | %dmV", 
					SOC_temperatureC / 1000, (SOC_temperatureC / 100) % 10, 
					Rotation_SpeedLevel_f * 100,
					realSOC_mV);
			}
		}
		else {	
			if (hosversionAtLeast(10,0,0)) {
				snprintf(skin_temperature_c, sizeof skin_temperature_c, 
					"%2.1f/%2.1f/%hu.%hhu\u00B0C@%+.1fW[%s]", 
					SOC_temperatureF, PCB_temperatureF, 
					skin_temperaturemiliC / 1000, (skin_temperaturemiliC / 100) % 10, 
					PowerConsumption, remainingBatteryLife);

				snprintf(skin_temperatureB_c, sizeof skin_temperatureB_c, 
					"%2.1f/%2.1f/%hu.%hhu\u00B0C(%2.1f%%)", 
					SOC_temperatureF, PCB_temperatureF, 
					skin_temperaturemiliC / 1000, (skin_temperaturemiliC / 100) % 10,
					Rotation_SpeedLevel_f * 100);

				snprintf(skin_temperatureS_c, sizeof skin_temperatureS_c, 
					"%2.1f\u00B0C(%2.0f%%)", 
					SOC_temperatureF,
					Rotation_SpeedLevel_f * 100);
			}
			else {
				snprintf(skin_temperature_c, sizeof skin_temperature_c, 
					"%hu.%hhu/%hu.%hhu/%hu.%hhu\u00B0C@%+.1fW[%s]", 
					SOC_temperatureC / 1000, (SOC_temperatureC / 100) % 10, 
					PCB_temperatureC / 1000, (PCB_temperatureC / 100) % 10, 
					skin_temperaturemiliC / 1000, (skin_temperaturemiliC / 100) % 10, 
					PowerConsumption, remainingBatteryLife);
					
				snprintf(skin_temperatureB_c, sizeof skin_temperatureB_c, 
					"%hu.%hhu/%hu.%hhu/%hu.%hhu\u00B0C(%2.1f%%)", 
					SOC_temperatureC / 1000, (SOC_temperatureC / 100) % 10, 
					PCB_temperatureC / 1000, (PCB_temperatureC / 100) % 10, 
					skin_temperaturemiliC / 1000, (skin_temperaturemiliC / 100) % 10,
					Rotation_SpeedLevel_f * 100);

				snprintf(skin_temperatureS_c, sizeof skin_temperatureS_c, 
					"%hu.%hhu\u00B0C(%2.0f%%)", 
					SOC_temperatureC / 1000, (SOC_temperatureC / 100) % 10, 
					Rotation_SpeedLevel_f * 100);
			}
		}
		mutexUnlock(&mutex_BatteryChecker);
		snprintf(Rotation_SpeedLevel_c, sizeof Rotation_SpeedLevel_c, "%2.1f%%", Rotation_Duty);
		
		///FPS
		snprintf(FPS_var_compressed_c, sizeof FPS_var_compressed_c, "%2.1f", FPSavg);

		mutexUnlock(&mutex_Misc);
		
		
		
	}
	virtual bool handleInput(uint64_t keysDown, uint64_t keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
		if (isKeyComboPressed(keysHeld, keysDown, mappedButtons)) {
			TeslaFPS = 60;
            if (skipMain)
                tsl::goBack();
            else {
			    tsl::setNextOverlay(filepath.c_str());
			    tsl::Overlay::get()->close();
            }
			return true;
		}
		return false;
	}
};