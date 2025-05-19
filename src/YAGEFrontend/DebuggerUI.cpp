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
        { 0x0000, 0x3FFF, IM_COL32(200,200,255,50), "ROM Bank 0" }, // ROM Bank 0
        { 0x4000, 0x7FFF, IM_COL32(180,200,255,50), "ROM Bank 1" }, // ROM Bank switchable
        { 0x8000, 0x9FFF, IM_COL32(200,255,200,50), "VRAM" }, // VRAM
        { 0xA000, 0xBFFF, IM_COL32(200,255,200,80), "External RAM" }, // External RAM
        { 0xC000, 0xCFFF, IM_COL32(255,200,200,50), "WRAM Bank 0" }, // WRAM bank0
        { 0xD000, 0xDFFF, IM_COL32(255,200,200,80), "WRAM Bank 1" }, // WRAM bank1
        { 0xE000, 0xFDFF, IM_COL32(200,200,200,50), "Echo Ram" }, // Echo RAM
        { 0xFE00, 0xFE9F, IM_COL32(255,200,255,50), "OAM" }, // OAM
        { 0xFEA0, 0xFEFF, IM_COL32(100,100,100,80), "UNUSABLE" }, // Not Usable
        { 0xFF00, 0xFF7F, IM_COL32(255,255,200,50), "IO Registers" }, // I/O
        { 0xFF80, 0xFFFE, IM_COL32(255,200,255,80), "HRAM" }, // HRAM
        { 0xFFFF, 0xFFFF, IM_COL32(255,200,255,80), "IE Register" }  // IE Register
    };

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
    if (ImGui::Button("Break into VS", ImVec2(120, 40))) DebuggerUtils::TriggerBreakpoint(nullptr);

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

            addRowPair("A", cpu.regA, prev.regA, "F", cpu.regF, prev.regF);
            addRowPair("B", cpu.regB, prev.regB, "C", cpu.regC, prev.regC);
            addRowPair("D", cpu.regD, prev.regD, "E", cpu.regE, prev.regE);
            addRowPair("H", cpu.regH, prev.regH, "L", cpu.regL, prev.regL);
            addRowPair("PC", cpu.regPC, prev.regPC, "SP", cpu.regSP, prev.regSP);
            ImGui::EndTable();
        }

        // Highlight halted flag change
        if (cpu.halted != prev.halted) ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
        ImGui::Text("Halted: %s", cpu.halted ? "Yes" : "No");
        if (cpu.halted != prev.halted) ImGui::PopStyleColor();
        // Highlight interrupt handling change
        if (cpu.handlingInterrupt != prev.handlingInterrupt) ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
        ImGui::Text("Handling Interrupt: %s", cpu.handlingInterrupt ? "Yes" : "No");
        if (cpu.handlingInterrupt != prev.handlingInterrupt) ImGui::PopStyleColor();
        ImGui::Text("Running: %s", cpu.running ? "Yes" : "No");
        ImGui::Text("Instr: %s", cpu.currentInstruction);
        ImGui::Text("Duration: %d cycles (Processed: %d)", cpu.instructionDurationCycles, cpu.cyclesProcessed);
    }

    // --- GPU State View ---
    if (ImGui::CollapsingHeader("GPU State", ImGuiTreeNodeFlags_DefaultOpen)) 
    {
        /*
        auto& gpu = data.gpu;
        if (ImGui::BeginTable("gpu_table", 2, ImGuiTableFlags_Borders)) 
        {
            ImGui::TableSetupColumn("MMIO"); ImGui::TableSetupColumn("Value");
            for (const auto& kv : gpu.mmio) {
                ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::Text("%s", kv.first.c_str());
                ImGui::TableNextColumn(); ImGui::Text("0x%02X", kv.second);
            }
            ImGui::EndTable();
        }
        ImGui::Text("Scanline: %d   Mode: %d", gpu.currentScanline, gpu.mode);
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

        if ( (m_state.m_selectedMemoryCell >= 0 || m_state.m_hoveredAddr >= 0) && mem)
        {
            ImGui::SameLine();

            int selectedMemory = m_state.m_selectedMemoryCell >= 0 ? m_state.m_selectedMemoryCell : m_state.m_hoveredAddr;
            ImGui::Text("Addr: 0x%04X", selectedMemory);
            ImGui::SameLine(); ImGui::Text("Value: 0x%02X", mem[selectedMemory]);
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
                    if (addr < EMULATOR_GB_MEMORY_SIZE && mem)
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
                    	sprintf_s(buf, "%02X", mem[addr]);

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
