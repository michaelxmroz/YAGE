#include "DebuggerUI.h"
#include "DebuggerUtils.h"
#include "imgui.h"

namespace
{
    struct Region
    {
        int start;
        int end;
        ImU32 color;
        const char* name;
    };
    const Region regions[] =
    {
        { 0x0000, 0x3FFF, IM_COL32(200,200,255,50), "ROM Bank 0" },
        { 0x4000, 0x7FFF, IM_COL32(180,200,255,50), "ROM Bank 1" },
        { 0x8000, 0x9FFF, IM_COL32(200,255,200,50), "VRAM" },
        { 0xA000, 0xBFFF, IM_COL32(200,255,200,80), "External RAM" },
        { 0xC000, 0xCFFF, IM_COL32(255,200,200,50), "WRAM Bank 0" },
        { 0xD000, 0xDFFF, IM_COL32(255,200,200,80), "WRAM Bank 1" },
        { 0xE000, 0xFDFF, IM_COL32(200,200,200,50), "Echo Ram" },
        { 0xFE00, 0xFE9F, IM_COL32(255,200,255,50), "OAM" },
        { 0xFEA0, 0xFEFF, IM_COL32(100,100,100,80), "UNUSABLE" },
        { 0xFF00, 0xFF7F, IM_COL32(255,255,200,50), "IO Registers" },
        { 0xFF80, 0xFFFE, IM_COL32(255,200,255,80), "HRAM" },
        { 0xFFFF, 0xFFFF, IM_COL32(255,200,255,80), "IE Register" }
    };

    struct PPURegisterInfo
    {
        uint32_t addr;
        const char* name;
        const char* description;
    };

    enum class PPURegister
    {
        LCDC = 0,
        STAT,
        SCY,
        SCX,
        LY,
        LYC,
        DMA,
        BGP,
        OBP0,
        OBP1,
        WY,
        WX,
        COUNT
    };

    const PPURegisterInfo PPURegisters[] =
    {
        { 0xFF40, "LCDC", "LCD Control" },
        { 0xFF41, "STAT", "LCD Status" },
        { 0xFF42, "SCY", "Background Scroll Y" },
        { 0xFF43, "SCX", "Background Scroll X" },
        { 0xFF44, "LY", "LCD Y line coordinate" },
        { 0xFF45, "LYC", "LCD Y Compare" },
        { 0xFF46, "DMA", "OAM DMA source address & start" },
        { 0xFF47, "BGP", "Background Palette Data" },
        { 0xFF48, "OBP0", "OBJ palette 0 data" },
        { 0xFF49, "OBP1", "OBJ palette 1 data" },
        { 0xFF4A, "WY", "Window position Y" },
        { 0xFF4B, "WX", "Window Position X" },
    };

    PPURegisterInfo GetPPURegInfo(PPURegister reg)
    {
        return PPURegisters[static_cast<int>(reg)];
    }

    // Helper for memory region tinting
    ImU32 GetMemRegionTint(int addr)
    {
        for (const auto& r : regions)
        {
            if (addr >= r.start && addr <= r.end)
            {
                return r.color;
            }
        }

        return IM_COL32(0, 0, 0, 0);
    }

    const char* GetMemRegionName(int addr)
    {
        for (const auto& r : regions)
        {
            if (addr >= r.start && addr <= r.end)
            {
                return r.name;
            }
        }
        return "";
    }

    uint8_t GetMem(uint8_t* mem, uint16_t addr)
    {
        if (!mem)
        {
            return 0;
        }

        return mem[addr];
    }

}



void DebuggerUI::Draw(EngineData& data)
{
    // Sync visibility
    m_state.m_showWindow = data.m_debuggerActive;
    if (!m_state.m_showWindow)
    {
        return;
    }

    // Bigger default size
    ImGui::SetNextWindowSize(ImVec2(800, 700), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Debugger", &m_state.m_showWindow)) { ImGui::End(); return; }

    // --- Control Row ---
    // Larger buttons
    ImGui::Dummy(ImVec2(0, 5));
    if (data.m_debuggerSteps == -1) 
    {
        if (ImGui::Button("Stop", ImVec2(100, 40))) data.m_debuggerSteps = 0;
    }
    else 
    {
        if (ImGui::Button("Run", ImVec2(100, 40))) data.m_debuggerSteps = -1;
    }
    ImGui::SameLine();
    if (ImGui::Button("Step Fwd", ImVec2(100, 40))) data.m_debuggerSteps++;
    ImGui::SameLine();
    if (ImGui::Button("Step Back", ImVec2(100, 40))) { /* TODO */ }
    ImGui::SameLine();
    if (ImGui::Button("Break into VS", ImVec2(120, 40))) data.m_triggerDebugBreak = true;

    ImGui::Spacing();
    ImGui::SetNextItemWidth(150);
    ImGui::InputInt("Cycles/Frames", &m_state.m_stepCount);
    ImGui::SameLine();
    if (ImGui::Button("Step N", ImVec2(80, 30))) data.m_debuggerSteps += m_state.m_stepCount;
    ImGui::Separator();

    // --- CPU State View ---
    if (ImGui::CollapsingHeader("CPU State", ImGuiTreeNodeFlags_DefaultOpen)) 
    {
        auto& cpu = data.m_cpuState;
        auto& prev = data.m_cpuStatePrevious;
        if (ImGui::BeginTable("cpu_table", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) 
        {
            ImGui::TableSetupColumn("Reg");
            ImGui::TableSetupColumn("Value");
            ImGui::TableSetupColumn("Reg");
            ImGui::TableSetupColumn("Value");
            ImGui::TableHeadersRow();

            auto addRowPair = [&](const char* n1, uint64_t v1, uint64_t p1, const char* n2, uint64_t v2, uint64_t p2)
        	{
                ImGui::TableNextRow();
                // First reg
                ImGui::TableNextColumn(); ImGui::Text("%s", n1);
                ImGui::TableNextColumn();
                if (v1 != p1) ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(255, 0, 0, 100));
                ImGui::Text("0x%0*llX", (p1 <= 0xFF) ? 2 : 4, v1);
                // Second reg
                ImGui::TableNextColumn(); ImGui::Text("%s", n2);
                ImGui::TableNextColumn();
                if (v2 != p2) ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(255, 0, 0, 100));
                ImGui::Text("0x%0*llX", (p2 <= 0xFF) ? 2 : 4, v2);
        	};

            addRowPair("A", cpu.m_regA, prev.m_regA, "F", cpu.m_regF, prev.m_regF);
            addRowPair("B", cpu.m_regB, prev.m_regB, "C", cpu.m_regC, prev.m_regC);
            addRowPair("D", cpu.m_regD, prev.m_regD, "E", cpu.m_regE, prev.m_regE);
            addRowPair("H", cpu.m_regH, prev.m_regH, "L", cpu.m_regL, prev.m_regL);
            addRowPair("PC", cpu.m_regPC, prev.m_regPC, "SP", cpu.m_regSP, prev.m_regSP);
            ImGui::EndTable();
        }

        // Highlight halted flag change
        if (cpu.m_halted != prev.m_halted) ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
        ImGui::Text("Halted: %s", cpu.m_halted ? "Yes" : "No");
        if (cpu.m_halted != prev.m_halted) ImGui::PopStyleColor();
        // Highlight interrupt handling change
        if (cpu.m_handlingInterrupt != prev.m_handlingInterrupt) ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
        ImGui::Text("Handling Interrupt: %s", cpu.m_handlingInterrupt ? "Yes" : "No");
        if (cpu.m_handlingInterrupt != prev.m_handlingInterrupt) ImGui::PopStyleColor();
        ImGui::Text("Running: %s", cpu.m_running ? "Yes" : "No");
        ImGui::Text("Instr: %s", cpu.m_currentInstruction);
        ImGui::Text("Duration: %d cycles (Processed: %d)", cpu.m_instructionDurationCycles, cpu.m_cyclesProcessed);
    }

    // --- PPU State View ---
    if (ImGui::CollapsingHeader("PPU State", ImGuiTreeNodeFlags_DefaultOpen))
    {
        uint8_t* mem = static_cast<uint8_t*>(data.m_rawMemoryView);

        int mode = data.m_ppuState.m_mode;
        const char* modeNames[] =
        {
            "Mode 0 - HBlank",
            "Mode 1 - VBlank",
            "Mode 2 - OAM",
            "Mode 3 - Drawing"
        };

        const char* modeLabel = GetMem(mem, PPURegisters[0].addr) & 0x80
            ? modeNames[mode] : "PPU Off";
        ImVec4 modeColor = (mode >= 0 && mode < 4)
            ? ImVec4(0.2f, 0.8f, 0.2f, 1.0f)
            : ImVec4(0.8f, 0.2f, 0.2f, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, modeColor);
        ImGui::Text("PPU: %s", modeLabel);
        ImGui::PopStyleColor();

        // PPU Registers: Show in 3 columns
        if (ImGui::BeginTable("ppu_regs", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
        {
            ImGui::TableSetupColumn("Reg"); ImGui::TableSetupColumn("Val");
            ImGui::TableSetupColumn("Reg"); ImGui::TableSetupColumn("Val");
            ImGui::TableSetupColumn("Reg"); ImGui::TableSetupColumn("Val");
            ImGui::TableHeadersRow();

            for (int i = 0; i < static_cast<int>(PPURegister::COUNT); i += 3)
            {
                ImGui::TableNextRow();
                for (int j = 0; j < 3; ++j)
                {
                    int index = i + j;
                    if (index >= static_cast<int>(PPURegister::COUNT)) break;
                    const auto& reg = PPURegisters[index];
                    ImGui::TableNextColumn(); ImGui::Text("%s", reg.name);
                    ImGui::TableNextColumn(); ImGui::Text("0x%02X", GetMem(mem, reg.addr));
                }
            }
            ImGui::EndTable();
        }

        // LCDC table
        ImGui::Separator();
        uint8_t lcdc = GetMem(mem, GetPPURegInfo(PPURegister::LCDC).addr);
        ImGui::Text("LCDC: 0x%02X", lcdc);
        if (ImGui::BeginTable("lcdc_table", 8, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
        {
            ImGui::TableSetupColumn("LCD/PPU");
            ImGui::TableSetupColumn("Window Map");
            ImGui::TableSetupColumn("Window");
            ImGui::TableSetupColumn("Tile Data");
            ImGui::TableSetupColumn("BG Map");
            ImGui::TableSetupColumn("OBJ Size");
            ImGui::TableSetupColumn("OBJ");
            ImGui::TableSetupColumn("BG/Win");
            ImGui::TableHeadersRow();

            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::Text("%s", (lcdc & 0x80) ? "On" : "Off");
            ImGui::TableNextColumn(); ImGui::Text("%s", (lcdc & 0x40) ? "9C00" : "9800");
            ImGui::TableNextColumn(); ImGui::Text("%s", (lcdc & 0x20) ? "On" : "Off");
            ImGui::TableNextColumn(); ImGui::Text("%s", (lcdc & 0x10) ? "8000" : "8800");
            ImGui::TableNextColumn(); ImGui::Text("%s", (lcdc & 0x08) ? "9C00" : "9800");
            ImGui::TableNextColumn(); ImGui::Text("%s", (lcdc & 0x04) ? "8x16" : "8x8");
            ImGui::TableNextColumn(); ImGui::Text("%s", (lcdc & 0x02) ? "On" : "Off");
            ImGui::TableNextColumn(); ImGui::Text("%s", (lcdc & 0x01) ? "On" : "Off");

            ImGui::EndTable();
        }

        // STAT table
        ImGui::Separator();
        uint8_t stat = GetMem(mem, GetPPURegInfo(PPURegister::STAT).addr);
        ImGui::Text("STAT: 0x%02X", stat);
        if (ImGui::BeginTable("stat_table", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
        {
            ImGui::TableSetupColumn("LYC Int");
            ImGui::TableSetupColumn("Mode 2 Int");
            ImGui::TableSetupColumn("Mode 1 Int");
            ImGui::TableSetupColumn("Mode 0 Int");
            ImGui::TableSetupColumn("LYC==LY");
            ImGui::TableSetupColumn("Mode");

            ImGui::TableHeadersRow();
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::Text("%s", (stat & 0x40) ? "On" : "Off");
            ImGui::TableNextColumn(); ImGui::Text("%s", (stat & 0x20) ? "On" : "Off");
            ImGui::TableNextColumn(); ImGui::Text("%s", (stat & 0x10) ? "On" : "Off");
            ImGui::TableNextColumn(); ImGui::Text("%s", (stat & 0x08) ? "On" : "Off");
            ImGui::TableNextColumn(); ImGui::Text("%s", (stat & 0x04) ? "Yes" : "No");

            // PPU Mode value or disabled fallback
            const char* statModeLabel = (lcdc & 0x80) ? modeNames[stat & 0x03] : "Disabled";
            ImGui::TableNextColumn(); ImGui::Text("%s", statModeLabel);

            ImGui::EndTable();
        }

        // 4) Internal PPU metrics
        /*
        ImGui::Separator();
        ImGui::Text("Line: %d    Pos X: %d", data.m_ppuState., data.gpu.dotPosition);
        ImGui::Text("Sprites Found: %d", data.gpu.spritesFound);
        ImGui::Text("FIFO Lengths: %d / %d", data.gpu.fifo0Size, data.gpu.fifo1Size);
        ImGui::Text("Fetcher State: %d", data.gpu.fetcherState);
        ImGui::Text("Cycles: Frame=%d  Line=%d  SinceMode=%d",
            data.gpu.cyclesThisFrame,
            data.gpu.cyclesThisLine,
            data.gpu.cyclesSinceMode);
            */
    }


    // --- Memory View ---
    if (ImGui::CollapsingHeader("Memory", ImGuiTreeNodeFlags_DefaultOpen)) 
    {
        float avail = ImGui::GetContentRegionAvail().x;
        float tableWidth = avail - ImGui::GetStyle().ScrollbarSize;
        float cellWidth = ImGui::CalcTextSize("FF").x + ImGui::GetStyle().CellPadding.x * 2;
        int cols = static_cast<int>((tableWidth - 80) / cellWidth);
        cols--;
        if (cols < 1) cols = 1;
        int totalCols = cols + 1;

        constexpr float MemoryViewHeight = 200.0f;


        uint16_t regionAddr = m_state.m_hoveredAddr >= 0 ? m_state.m_hoveredAddr : m_state.m_memoryFirstVisibleAddr;
        regionAddr = m_state.m_selectedMemoryCell >= 0 ? m_state.m_selectedMemoryCell : regionAddr;

        ImU32 regionColor = GetMemRegionTint(regionAddr);
        const char* regionLabel = GetMemRegionName(regionAddr);


        ImVec2 labelSize = ImGui::CalcTextSize(regionLabel);
        ImVec2 cursor = ImGui::GetCursorScreenPos();
        ImVec2 padding = ImGui::GetStyle().FramePadding;
        ImVec2 labelRectMin = cursor;
        ImVec2 labelRectMax = ImVec2(cursor.x + labelSize.x + padding.x * 2, cursor.y + labelSize.y + padding.y * 2);

        // Draw background rectangle with region color
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddRectFilled(labelRectMin, labelRectMax, regionColor, 4.0f);

        // Draw the label text centered inside the rectangle
        ImGui::SetCursorScreenPos(ImVec2(cursor.x + padding.x, cursor.y + padding.y));
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32_WHITE);
        ImGui::TextUnformatted(regionLabel);
        ImGui::PopStyleColor();

        uint8_t* mem = static_cast<uint8_t*>(data.m_rawMemoryView);

        if ( (m_state.m_selectedMemoryCell >= 0 || m_state.m_hoveredAddr >= 0))
        {
            ImGui::SameLine();

            int selectedMemory = m_state.m_selectedMemoryCell >= 0 ? m_state.m_selectedMemoryCell : m_state.m_hoveredAddr;
            ImGui::Text("Addr: 0x%04X", selectedMemory);
            ImGui::SameLine(); ImGui::Text("Value: 0x%02X", GetMem(mem, selectedMemory));
            ImGui::SameLine();
            if (ImGui::Button("Add Data BP", ImVec2(50, 10)))
            {
                /* TODO: Add data breakpoint for m_data.selectedMemAddr */
            }
        }

        ImGui::BeginChild("MemoryView", ImVec2(0, MemoryViewHeight), false);

        float scroll = ImGui::GetScrollY();
        float rowHeight = ImGui::GetTextLineHeightWithSpacing();
        int firstVisibleRow = static_cast<int>(scroll / rowHeight);
        m_state.m_memoryFirstVisibleAddr = firstVisibleRow * cols;


        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

        if (ImGui::BeginTable("mem_table", totalCols, ImGuiTableFlags_RowBg |
            ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInner |
            ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_HighlightHoveredColumn, ImVec2(tableWidth, 0)))
        {
            ImGui::TableSetupColumn("Addr", ImGuiTableColumnFlags_None, 55);

            bool foundHover = false;

            for (int i = 0; i < cols; ++i)
            {
                ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_None, cellWidth);
            }

            ImGui::TableHeadersRow();

            for (int rowBase = 0; rowBase < EMULATOR_GB_MEMORY_SIZE; rowBase += cols)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5);
                ImGui::Text("0x%04X", rowBase);

                for (int c = 0; c < cols; ++c) 
                {
                    int addr = rowBase + c;
                    ImGui::TableNextColumn();
                    if (addr < EMULATOR_GB_MEMORY_SIZE)
                    {
                        // Region-based tint
                        ImU32 tint = GetMemRegionTint(addr);
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, tint);
                        // Highlight selected cell
                        if (addr == m_state.m_selectedMemoryCell)
                        {
                            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(0, 255, 0, 100));
                        }

                        ImGui::PushID(addr);

                        char buf[8];
                    	sprintf_s(buf, "%02X", GetMem(mem, addr));

                        // Transparent button background
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

                        if (ImGui::Button(buf, ImVec2(ImGui::GetColumnWidth(), ImGui::GetTextLineHeightWithSpacing()))) 
                        {
                            if(m_state.m_selectedMemoryCell == addr)
                            {
                                m_state.m_selectedMemoryCell = -1;
                            }
                            else
                            {
                                m_state.m_selectedMemoryCell = addr;
                            }
                        }

                        if (ImGui::IsItemHovered()) 
                        {
                            m_state.m_hoveredAddr = addr;
                            foundHover = true;
                        }

                        ImGui::PopStyleColor(3);
                        ImGui::PopID();

                    }
                    else 
                    {
                        ImGui::Text("--");
                    }
                }
            }

            if(!foundHover)
            {
                m_state.m_hoveredAddr = -1;
            }

            ImGui::EndTable();
        }
        ImGui::PopStyleVar(2);
        ImGui::EndChild();
    }

    // --- Breakpoints ---
    if (ImGui::CollapsingHeader("Breakpoints", ImGuiTreeNodeFlags_DefaultOpen)) 
    {
        /*
        ImGui::Text("Execution Breakpoint");
        ImGui::SetNextItemWidth(120);
        ImGui::InputInt("PC Addr (hex)", &ui.execAddr, 1, 0, ImGuiInputTextFlags_CharsHexadecimal);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(80);
        ImGui::InputInt("Count", &ui.execCount);
        if (ImGui::Button("Add Exec BP", ImVec2(100, 30))) {  }
        ImGui::Separator();
        ImGui::Text("Data Breakpoint");
        ImGui::SetNextItemWidth(120);
        ImGui::InputInt("Mem Addr (hex)", &ui.dataAddr, 1, 0, ImGuiInputTextFlags_CharsHexadecimal);
        if (ImGui::Button("Add Data BP", ImVec2(100, 30))) {  }
    */
    }

    ImGui::End();
}

void DebuggerUI::Toggle(EngineData& data)
{
    m_state.m_showWindow = !m_state.m_showWindow;
    data.m_debuggerActive = m_state.m_showWindow;
}
