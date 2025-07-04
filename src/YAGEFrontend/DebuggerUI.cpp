#include "DebuggerUI.h"
#include "DebuggerUtils.h"
#include "imgui.h"
#include "Emulator.h"
#include <cstdlib>
#include <algorithm>
#include <string>

#if defined (DEBUGGER)

namespace
{
    // Theme colors
    namespace Theme
    {
        const ImVec4 Background = ImVec4(0.12f, 0.12f, 0.18f, 1.00f);  // #1E1E2E
        const ImVec4 Panel = ImVec4(0.16f, 0.16f, 0.21f, 1.00f);      // #282A36
        const ImVec4 Accent = ImVec4(0.40f, 0.40f, 0.80f, 1.00f);     // #6272A4
        const ImVec4 Success = ImVec4(0.40f, 0.80f, 0.40f, 1.00f);    // #50FA7B
        const ImVec4 Warning = ImVec4(0.80f, 0.40f, 0.40f, 1.00f);    // #FF5555
        const ImVec4 Text = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);       // #F8F8F2
        const ImVec4 TextDim = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);    // #6272A4

        // Button states
        const ImVec4 RunButton = ImVec4(0.40f, 0.80f, 0.40f, 1.00f);  // Green
        const ImVec4 StepButton = ImVec4(0.40f, 0.40f, 0.80f, 1.00f); // Blue
        const ImVec4 PauseButton = ImVec4(0.80f, 0.80f, 0.40f, 1.00f); // Yellow
        const ImVec4 BreakButton = ImVec4(0.80f, 0.40f, 0.40f, 1.00f); // Red

        // Spacing constants
        const float Padding = 8.0f;
        const float Spacing = 8.0f;
        const float SectionSpacing = 16.0f;
    }

    // Debugger constants
    constexpr int PC_MEMORY_VIEW_RANGE = 5;  // Show this many bytes before and after PC

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

    struct PPUModeInfo 
    {
        const char* name;
        ImVec4 color;
    };

    enum class PPUMode
    {
        HBlank = 0,
        VBlank,
        OAMScan,
        Drawing
    };

    const PPUModeInfo ppuModes[] = 
    {
        { "HBlank", ImVec4(0.0f, 0.4f, 0.0f, 1.0f) },   // Dark Green
        { "VBlank", ImVec4(0.0f, 0.6f, 0.6f, 1.0f) },   // Teal
        { "OAM Scan", ImVec4(0.5f, 0.5f, 0.0f, 1.0f) }, // Dark Yellow
        { "Drawing", ImVec4(1.0f, 0.55f, 0.0f, 1.0f) }, // Orange
    };

    const char* breakpointTypes[] = { "PC", "Instruction", "Count", "Memory" };

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


    void AddBreakpoint(DebuggerState& data, Emulator* emulator, DebuggerState::BreakpointType type, uint32_t value)
    {
        // Check if breakpoint already exists
        for (const auto& bp : data.m_breakpoints)
        {
            if (bp.m_type == type && bp.m_value == value)
            {
                return; // Breakpoint already exists
            }
        }

        // Add new breakpoint
        DebuggerState::Breakpoint newBp(type, value, data.m_nextBreakpointId++);
        data.m_breakpoints.push_back(newBp);

        // Register with emulator
        switch (type)
        {
        case DebuggerState::BreakpointType::PC:
            emulator->SetPCCallback(static_cast<uint16_t>(value), &DebuggerUtils::TriggerDebuggerBreakpoint, &data);
            break;
        case DebuggerState::BreakpointType::Instruction:
            emulator->SetInstructionCallback(static_cast<uint8_t>(value), &DebuggerUtils::TriggerDebuggerBreakpoint, &data);
            break;
        case DebuggerState::BreakpointType::InstructionCount:
            emulator->SetInstructionCountCallback(value, &DebuggerUtils::TriggerDebuggerBreakpoint, &data);
            break;
        case DebuggerState::BreakpointType::MemoryWrite:
            emulator->SetDataCallback(static_cast<uint16_t>(value), &DebuggerUtils::TriggerDebuggerBreakpoint, &data);
            break;
        }
    }

    void RemoveBreakpoint(DebuggerState& data, Emulator* emulator, int breakpointId)
    {
        // Find and remove the breakpoint
        auto it = std::find_if(data.m_breakpoints.begin(), data.m_breakpoints.end(),
            [breakpointId](const DebuggerState::Breakpoint& bp) { return bp.m_id == breakpointId; });

        if (it != data.m_breakpoints.end())
        {
            // Clear the callback from emulator
            switch (it->m_type)
            {
            case DebuggerState::BreakpointType::PC:
                emulator->SetPCCallback(static_cast<uint16_t>(it->m_value), nullptr, nullptr);
                break;
            case DebuggerState::BreakpointType::Instruction:
                emulator->SetInstructionCallback(static_cast<uint8_t>(it->m_value), nullptr, nullptr);
                break;
            case DebuggerState::BreakpointType::InstructionCount:
                emulator->SetInstructionCountCallback(it->m_value, nullptr, nullptr);
                break;
            case DebuggerState::BreakpointType::MemoryWrite:
                emulator->SetDataCallback(static_cast<uint16_t>(it->m_value), nullptr, nullptr);
                break;
            }

            data.m_breakpoints.erase(it);
        }
    }

    void UpdateBreakpoints(DebuggerState& data, Emulator* emulator)
    {
        // Clear all existing callbacks
        emulator->ClearCallbacks();

        // Re-register all enabled breakpoints
        for (const auto& bp : data.m_breakpoints)
        {
            if (bp.m_enabled)
            {
                switch (bp.m_type)
                {
                case DebuggerState::BreakpointType::PC:
                    emulator->SetPCCallback(static_cast<uint16_t>(bp.m_value), &DebuggerUtils::TriggerDebuggerBreakpoint, &data);
                    break;
                case DebuggerState::BreakpointType::Instruction:
                    emulator->SetInstructionCallback(static_cast<uint8_t>(bp.m_value), &DebuggerUtils::TriggerDebuggerBreakpoint, &data);
                    break;
                case DebuggerState::BreakpointType::InstructionCount:
                    emulator->SetInstructionCountCallback(bp.m_value, &DebuggerUtils::TriggerDebuggerBreakpoint, &data);
                    break;
                case DebuggerState::BreakpointType::MemoryWrite:
                    emulator->SetDataCallback(static_cast<uint16_t>(bp.m_value), &DebuggerUtils::TriggerDebuggerBreakpoint, &data);
                    break;
                }
            }
        }
    }

    void ToggleBreakpoint(DebuggerState& data, Emulator* emulator, int breakpointId)
    {
        // Find the breakpoint
        auto it = std::find_if(data.m_breakpoints.begin(), data.m_breakpoints.end(),
            [breakpointId](const DebuggerState::Breakpoint& bp) { return bp.m_id == breakpointId; });

        if (it != data.m_breakpoints.end())
        {
            // Toggle the enabled state
            it->m_enabled = !it->m_enabled;

            // Update the callback in the emulator
            if (it->m_enabled)
            {
                // Re-register the callback
                switch (it->m_type)
                {
                case DebuggerState::BreakpointType::PC:
                    emulator->SetPCCallback(static_cast<uint16_t>(it->m_value), &DebuggerUtils::TriggerDebuggerBreakpoint, &data);
                    break;
                case DebuggerState::BreakpointType::Instruction:
                    emulator->SetInstructionCallback(static_cast<uint8_t>(it->m_value), &DebuggerUtils::TriggerDebuggerBreakpoint, &data);
                    break;
                case DebuggerState::BreakpointType::InstructionCount:
                    emulator->SetInstructionCountCallback(it->m_value, &DebuggerUtils::TriggerDebuggerBreakpoint, &data);
                    break;
                case DebuggerState::BreakpointType::MemoryWrite:
                    emulator->SetDataCallback(static_cast<uint16_t>(it->m_value), &DebuggerUtils::TriggerDebuggerBreakpoint, &data);
                    break;
                }
            }
            else
            {
                // Remove the callback
                switch (it->m_type)
                {
                case DebuggerState::BreakpointType::PC:
                    emulator->SetPCCallback(static_cast<uint16_t>(it->m_value), nullptr, nullptr);
                    break;
                case DebuggerState::BreakpointType::Instruction:
                    emulator->SetInstructionCallback(static_cast<uint8_t>(it->m_value), nullptr, nullptr);
                    break;
                case DebuggerState::BreakpointType::InstructionCount:
                    emulator->SetInstructionCountCallback(it->m_value, nullptr, nullptr);
                    break;
                case DebuggerState::BreakpointType::MemoryWrite:
                    emulator->SetDataCallback(static_cast<uint16_t>(it->m_value), nullptr, nullptr);
                    break;
                }
            }
        }
    }

    bool HasBreakpointAtAddress(const DebuggerState& data, uint16_t address)
    {
        for (const auto& bp : data.m_breakpoints)
        {
            if (bp.m_enabled && bp.m_type == DebuggerState::BreakpointType::MemoryWrite && bp.m_value == address)
            {
                return true;
            }
        }
        return false;
    }


}

void DrawCPUState(const DebuggerState& data, Emulator* emulator)
{
    auto& cpu = data.m_cpuState;
    auto& prev = data.m_cpuStatePrevious;
    uint8_t* mem = static_cast<uint8_t*>(data.m_rawMemoryView);

    auto currentDisasm = emulator->GetDisassemblyInfo(cpu.m_currentInstructionAddr);

    // Parent table to contain all three sections
    if (ImGui::BeginTable("cpu_state_layout", 3, ImGuiTableFlags_None))
    {
        ImGui::TableSetupColumn("Registers", ImGuiTableColumnFlags_WidthFixed, 200);
        ImGui::TableSetupColumn("PC Memory", ImGuiTableColumnFlags_WidthFixed, 400);
        ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 150);
        ImGui::TableNextRow();

        // Register pairs table
        ImGui::TableNextColumn();
        if (ImGui::BeginTable("cpu_table", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) 
        {
            ImGui::TableSetupColumn("Reg", ImGuiTableColumnFlags_WidthFixed, 30);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, 60);
            ImGui::TableSetupColumn("Reg", ImGuiTableColumnFlags_WidthFixed, 30);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, 60);
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

        // Memory view around PC
        ImGui::TableNextColumn();
        if (ImGui::BeginTable("pc_mem", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
        {
            uint16_t pc = cpu.m_regPC;
            
            ImGui::TableSetupColumn("Addr", ImGuiTableColumnFlags_WidthFixed, 80);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, 60);
            ImGui::TableSetupColumn("Instruction", ImGuiTableColumnFlags_WidthFixed, 80);
            ImGui::TableSetupColumn("Operand", ImGuiTableColumnFlags_WidthFixed, 60);
            ImGui::TableHeadersRow();
            
            // Find the base address of the current instruction
            
            uint16_t currentBaseAddr = currentDisasm.baseAddr;
            
            // Create a single vector for all disassembly entries, pre-populated with empty entries
            std::vector<Emulator::DisassemblyInfo> disasmEntries(PC_MEMORY_VIEW_RANGE * 2 + 1, {"", 0, 0, 0xFFFF});
            
            // Set the current instruction in the middle
            disasmEntries[PC_MEMORY_VIEW_RANGE] = currentDisasm;
            
            // Find previous instructions
            uint16_t addr = currentBaseAddr;
            for (int i = PC_MEMORY_VIEW_RANGE - 1; i >= 0; --i)
            {
                if (addr == 0) 
                {
                    break;
                }
                addr--;
                auto disasm = emulator->GetDisassemblyInfo(addr);
                addr = disasm.baseAddr;
                disasmEntries[i] = disasm;
            }

            // Find next instructions
            addr = currentBaseAddr + currentDisasm.size;
            for (int i = PC_MEMORY_VIEW_RANGE + 1; i < disasmEntries.size(); ++i)
            {
                if (addr >= 0xFFFF) 
                {       
                    break;
                }
                auto disasm = emulator->GetDisassemblyInfo(addr);
                addr = disasm.baseAddr;
                disasmEntries[i] = disasm;
                addr += disasm.size;
            }
            
            // Display all instructions
            for (const auto& disasm : disasmEntries)
            {
                ImGui::TableNextRow();
                
                // Address column
                ImGui::TableNextColumn();
                if (disasm.baseAddr != 0xFFFF)
                {
                    ImGui::Text("0x%04X", disasm.baseAddr);
                    
                    // Value column
                    ImGui::TableNextColumn();
                    ImGui::Text("%02X", GetMem(mem, disasm.baseAddr));

                    // Disassembly column
                    ImGui::TableNextColumn();
                    if (disasm.baseAddr == currentBaseAddr)
                    {
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(255, 255, 0, 100));
                    }
                    ImGui::Text("%s", disasm.mnemonic);
                    
                    // Operand column
                    ImGui::TableNextColumn();
                    if (disasm.baseAddr != 0xFFFF)
                    {
                        if (disasm.size == 1 || GetMem(mem,disasm.baseAddr) == 0xCB)
                        {
                            ImGui::Text("-");
                        }
                        else if (disasm.size == 2)
                        {
                            ImGui::Text("0x%02X", GetMem(mem, disasm.baseAddr + 1));
                        }
                        else if (disasm.size == 3)
                        {
                            uint16_t operand = GetMem(mem, disasm.baseAddr + 1) | (GetMem(mem, disasm.baseAddr + 2) << 8);
                            ImGui::Text("0x%04X", operand);
                        }
                    }
                    else
                    {
                        ImGui::Text("--");
                    }
                }
                else
                {
                    ImGui::Text("--");
                    ImGui::TableNextColumn();
                    ImGui::Text("--");
                    ImGui::TableNextColumn();
                    ImGui::Text("--");
                    ImGui::TableNextColumn();
                    ImGui::Text("--");
                }
            }
            ImGui::EndTable();
        }

        // CPU Status column
        ImGui::TableNextColumn();
        if (cpu.m_halted != prev.m_halted) ImGui::PushStyleColor(ImGuiCol_Text, Theme::Warning);
        ImGui::Text("Halted: %s", cpu.m_halted ? "Yes" : "No");
        if (cpu.m_halted != prev.m_halted) ImGui::PopStyleColor();

        if (cpu.m_handlingInterrupt != prev.m_handlingInterrupt) ImGui::PushStyleColor(ImGuiCol_Text, Theme::Warning);
        ImGui::Text("Handling Interrupt: %s", cpu.m_handlingInterrupt ? "Yes" : "No");
        if (cpu.m_handlingInterrupt != prev.m_handlingInterrupt) ImGui::PopStyleColor();

        ImGui::Text("Running: %s", cpu.m_running ? "Yes" : "No");
        ImGui::Text("Instr: %s", currentDisasm.mnemonic);
        ImGui::Text("Duration: %d cycles", cpu.m_instructionDurationCycles);
        ImGui::Text("Processed: %d", cpu.m_cyclesProcessed);

        ImGui::EndTable();
    }
}

void DrawPPUBar(const Emulator::PPUState& ppuState)
{
    constexpr int totalCyclesPerLine = 456;
    constexpr int OAMDuration = 80;
    int mode = ppuState.m_mode;
    int cyclesInLine = ppuState.m_cyclesInLine;
    int cyclesInMode = ppuState.m_cyclesInMode;

    // Calculate phase durations so far
    int oamCycles = std::min(cyclesInLine, OAMDuration);
    int drawCycles = 0;
    int hblankCycles = 0;

    if (mode == static_cast<int>(PPUMode::Drawing))
    {
        drawCycles = cyclesInLine - OAMDuration;
    }
    else if (mode == static_cast<int>(PPUMode::HBlank))
    {
        hblankCycles = cyclesInMode;
        drawCycles = cyclesInLine - OAMDuration - cyclesInMode;
    }

    // Status text with icons
    ImGui::Text("Line: %d | Total: %d | OAM: %d | Draw: %d | HBlank: %d", 
        ppuState.m_lineY, cyclesInLine, oamCycles, drawCycles, hblankCycles);

    // Draw segmented progress bar
    float fullWidth = ImGui::GetContentRegionAvail().x;
    float barHeight = ImGui::GetTextLineHeight() * 1.2f;

    ImVec2 barStart = ImGui::GetCursorScreenPos();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    auto drawSegment = [&](float startX, float width, ImVec4 color) 
    {
        drawList->AddRectFilled(
            ImVec2(startX, barStart.y),
            ImVec2(startX + width, barStart.y + barHeight),
            ImColor(color)
        );
    };

    if (mode == static_cast<int>(PPUMode::VBlank)) 
    {
        // Entire bar is VBlank
        drawSegment(barStart.x, fullWidth * (static_cast<float>(cyclesInLine) / totalCyclesPerLine), ppuModes[static_cast<int>(PPUMode::VBlank)].color);
    }
    else 
    {
        float scale = fullWidth / static_cast<float>(totalCyclesPerLine);
        float pos = barStart.x;

        if (oamCycles > 0) {
            float w = oamCycles * scale;
            drawSegment(pos, w, ppuModes[static_cast<int>(PPUMode::OAMScan)].color); 
            pos += w;
        }

        if (drawCycles > 0) {
            float w = drawCycles * scale;
            drawSegment(pos, w, ppuModes[static_cast<int>(PPUMode::Drawing)].color); 
            pos += w;
        }

        if (hblankCycles > 0) {
            float w = hblankCycles * scale;
            drawSegment(pos, w, ppuModes[static_cast<int>(PPUMode::HBlank)].color); 
            pos += w;
        }
    }

    // Draw border with rounded corners
    drawList->AddRect(
        barStart,
        ImVec2(barStart.x + fullWidth, barStart.y + barHeight),
        IM_COL32(255, 255, 255, 255),
        4.0f  // Rounding
    );

    ImGui::Dummy(ImVec2(fullWidth, barHeight + ImGui::GetStyle().ItemSpacing.y));
}

void DrawFIFOBars(const Emulator::FIFOSizes& fifoSizes)
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    float barHeight = ImGui::GetTextLineHeight() * 0.8f;
    float fullWidth = ImGui::GetContentRegionAvail().x;
    ImU32 borderColor = IM_COL32(255, 255, 255, 255);

    // Calculate layout - labels take up ~30% of width, bars take up the rest
    float labelWidth = fullWidth * 0.3f;
    float barWidth = (fullWidth - labelWidth - ImGui::GetStyle().ItemSpacing.x * 3) * 0.5f;

    // Single line layout: BG Label | BG Bar | Sprite Label | Sprite Bar
    ImGui::Text("BG FIFO: %u/16", fifoSizes.m_backgroundFIFOCount);
    ImGui::SameLine();
    
    // Background FIFO bar
    ImVec2 bgBarStart = ImGui::GetCursorScreenPos();
    float bgFilledWidth = (static_cast<float>(fifoSizes.m_backgroundFIFOCount) / 16.0f) * barWidth;
    drawList->AddRectFilled(bgBarStart, ImVec2(bgBarStart.x + bgFilledWidth, bgBarStart.y + barHeight), IM_COL32(50, 150, 50, 255));
    drawList->AddRect(bgBarStart, ImVec2(bgBarStart.x + barWidth, bgBarStart.y + barHeight), borderColor, 4.0f);
    
    // Move cursor past the background bar
    ImGui::SetCursorScreenPos(ImVec2(bgBarStart.x + barWidth + ImGui::GetStyle().ItemSpacing.x, bgBarStart.y));
    ImGui::Text("Sprite FIFO: %u/16", fifoSizes.m_spriteFIFOCount);
    ImGui::SameLine();
    
    // Sprite FIFO bar
    ImVec2 spriteBarStart = ImGui::GetCursorScreenPos();
    float spriteFilledWidth = (static_cast<float>(fifoSizes.m_spriteFIFOCount) / 16.0f) * barWidth;
    drawList->AddRectFilled(spriteBarStart, ImVec2(spriteBarStart.x + spriteFilledWidth, spriteBarStart.y + barHeight), IM_COL32(50, 50, 150, 255));
    drawList->AddRect(spriteBarStart, ImVec2(spriteBarStart.x + barWidth, spriteBarStart.y + barHeight), borderColor, 4.0f);
    
    // Move cursor to next line
    ImGui::SetCursorScreenPos(ImVec2(bgBarStart.x, bgBarStart.y + barHeight + ImGui::GetStyle().ItemSpacing.y));
}

void DebuggerUI::Draw(DebuggerState& data, Emulator* emulator)
{
    // Sync visibility
    m_state.m_showWindow = data.m_debuggerActive;
    if (!m_state.m_showWindow)
    {
        return;
    }

    // Update breakpoints to ensure they're registered with the emulator
    UpdateBreakpoints(data, emulator);

    // Set window style
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(Theme::Padding, Theme::Padding));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(Theme::Spacing, Theme::Spacing));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(Theme::Padding, Theme::Padding));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);

    // Set colors
    ImGui::PushStyleColor(ImGuiCol_WindowBg, Theme::Background);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::Panel);
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::Text);
    ImGui::PushStyleColor(ImGuiCol_TextDisabled, Theme::TextDim);
    ImGui::PushStyleColor(ImGuiCol_Header, Theme::Accent);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(Theme::Accent.x, Theme::Accent.y, Theme::Accent.z, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(Theme::Accent.x, Theme::Accent.y, Theme::Accent.z, 0.6f));

    // Bigger default size
    ImGui::SetNextWindowSize(ImVec2(800, 700), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Debugger", &m_state.m_showWindow)) 
    { 
        ImGui::PopStyleColor(7);
        ImGui::PopStyleVar(6);
        ImGui::End(); 
        return; 
    }

    // --- Control Bar ---
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(Theme::Spacing * 2, Theme::Spacing));
        
        // Run/Stop button
        if (data.m_debuggerSteps == -1) 
        {
            ImGui::PushStyleColor(ImGuiCol_Button, Theme::PauseButton);
            if (ImGui::Button("Stop", ImVec2(100, 40))) data.m_debuggerSteps = 0;
        }
        else 
        {
            ImGui::PushStyleColor(ImGuiCol_Button, Theme::RunButton);
            if (ImGui::Button("Run", ImVec2(100, 40))) data.m_debuggerSteps = -1;
        }
        ImGui::PopStyleColor();
        ImGui::SameLine();

        // Step buttons
        ImGui::PushStyleColor(ImGuiCol_Button, Theme::StepButton);
        if (ImGui::Button("Step", ImVec2(100, 40))) data.m_debuggerSteps++;
        ImGui::SameLine();
        if (ImGui::Button("Back", ImVec2(100, 40))) { /* TODO */ }
        ImGui::SameLine();
        ImGui::PopStyleColor();

        // Break button
        ImGui::PushStyleColor(ImGuiCol_Button, Theme::BreakButton);
        if (ImGui::Button("Break", ImVec2(120, 40))) data.m_triggerDebugBreak = true;
        ImGui::PopStyleColor();

        // Cycle/Frame selector
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150);
        ImGui::InputInt("Cycles", &m_state.m_stepCount);
        ImGui::SameLine();
        if (ImGui::Button("Step N", ImVec2(80, 30))) data.m_debuggerSteps += m_state.m_stepCount;
        ImGui::SameLine();
        if (ImGui::Button(data.m_microstepping ? "tCycles" : "mCycles", ImVec2(80, 30))) data.m_microstepping = !data.m_microstepping;

        if (data.m_microstepping)
        {
            ImGui::SameLine();
            ImGui::Text("%d/4", data.m_tCyclesStepped + 1);
        }

        ImGui::PopStyleVar();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // --- CPU State View ---
    if (ImGui::CollapsingHeader("CPU State", ImGuiTreeNodeFlags_DefaultOpen)) 
    {
        DrawCPUState(data, emulator);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // --- PPU State View ---
    if (ImGui::CollapsingHeader("PPU State", ImGuiTreeNodeFlags_DefaultOpen))
    {
        uint8_t* mem = static_cast<uint8_t*>(data.m_rawMemoryView);
        int mode = data.m_ppuState.m_mode;
        bool lcdEnabled = GetMem(mem, PPURegisters[0].addr) & 0x80;

        const char* modeLabel = lcdEnabled ? ppuModes[mode].name : "Off";
        ImVec4 modeColor = lcdEnabled ? ppuModes[mode].color : Theme::TextDim;

        ImGui::PushStyleColor(ImGuiCol_Text, modeColor);
        ImGui::Text("PPU: %s", modeLabel);
        ImGui::PopStyleColor();

        DrawPPUBar(data.m_ppuState);
        ImGui::Spacing();

        // PPU Registers in a compact grid
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

        ImGui::Spacing();
        DrawFIFOBars(data.m_ppuState.m_fifoSizes);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // --- Breakpoints Section ---
    if (ImGui::CollapsingHeader("Breakpoints", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Unified breakpoint input form
        ImGui::Text("Add Breakpoint:");
        ImGui::SameLine();
        
        // Type dropdown
        
        ImGui::SetNextItemWidth(100);
        if (ImGui::Combo("##Type", &m_state.m_selectedBreakpointType, breakpointTypes, IM_ARRAYSIZE(breakpointTypes)))
        {
            // Clear input when type changes
            m_state.m_unifiedBreakpointInput[0] = '\0';
            m_state.m_breakpointInputError = false;
        }
        ImGui::SameLine();
        
        // Value input with validation
        ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_CharsHexadecimal;
        if (m_state.m_selectedBreakpointType == static_cast<int>(DebuggerState::BreakpointType::InstructionCount)) // Count type
        {
            inputFlags = ImGuiInputTextFlags_CharsDecimal;
        }
        
        ImGui::SetNextItemWidth(120);
        if (ImGui::InputText("##Value", m_state.m_unifiedBreakpointInput, IM_ARRAYSIZE(m_state.m_unifiedBreakpointInput), inputFlags))
        {
            m_state.m_breakpointInputError = false;
        }
        ImGui::SameLine();
        
        // Add button
        if (ImGui::Button("Add Breakpoint"))
        {
            bool validInput = true;
            uint32_t value = 0;
            
            // Parse and validate input based on type
            switch (m_state.m_selectedBreakpointType)
            {
            case 0: // PC
            case 3: // Memory
                {
                    char* endPtr;
                    value = static_cast<uint32_t>(strtoul(m_state.m_unifiedBreakpointInput, &endPtr, 16));
                    if (endPtr == m_state.m_unifiedBreakpointInput || *endPtr != '\0' || value > 0xFFFF)
                    {
                        validInput = false;
                        strcpy_s(m_state.m_breakpointErrorMessage, "Invalid hex address (0x0000-0xFFFF)");
                    }
                }
                break;
                
            case 1: // Instruction
                {
                    char* endPtr;
                    value = static_cast<uint32_t>(strtoul(m_state.m_unifiedBreakpointInput, &endPtr, 16));
                    if (endPtr == m_state.m_unifiedBreakpointInput || *endPtr != '\0' || value > 0xFF)
                    {
                        validInput = false;
                        strcpy_s(m_state.m_breakpointErrorMessage, "Invalid hex instruction (0x00-0xFF)");
                    }
                }
                break;
                
            case 2: // Count
                {
                    char* endPtr;
                    value = static_cast<uint32_t>(strtoull(m_state.m_unifiedBreakpointInput, &endPtr, 10));
                    if (endPtr == m_state.m_unifiedBreakpointInput || *endPtr != '\0' || value == 0)
                    {
                        validInput = false;
                        strcpy_s(m_state.m_breakpointErrorMessage, "Invalid count (positive integer)");
                    }
                }
                break;
            }
            
            if (validInput)
            {
                DebuggerState::BreakpointType type = static_cast<DebuggerState::BreakpointType>(m_state.m_selectedBreakpointType);
                AddBreakpoint(data, emulator, type, value);
                m_state.m_unifiedBreakpointInput[0] = '\0'; // Clear input
                m_state.m_breakpointInputError = false;
            }
            else
            {
                m_state.m_breakpointInputError = true;
            }
        }
        
        // Show error message if validation failed
        if (m_state.m_breakpointInputError)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, Theme::Warning);
            ImGui::Text("%s", m_state.m_breakpointErrorMessage);
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();

        // Active breakpoints table
        if (ImGui::BeginTable("breakpoints_table", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
        {
            ImGui::TableSetupColumn("Type");
            ImGui::TableSetupColumn("Value");
            ImGui::TableSetupColumn("Enabled");
            ImGui::TableSetupColumn("Action");
            ImGui::TableHeadersRow();

            for (const auto& bp : data.m_breakpoints)
            {
                ImGui::TableNextRow();
                
                // Type column
                ImGui::TableNextColumn();

                ImGui::Text("%s", breakpointTypes[static_cast<int>(bp.m_type)]);
                
                // Value column
                ImGui::TableNextColumn();
                switch (bp.m_type)
                {
                case DebuggerState::BreakpointType::PC:
                case DebuggerState::BreakpointType::MemoryWrite:
                    ImGui::Text("0x%04X", bp.m_value);
                    break;
                case DebuggerState::BreakpointType::Instruction:
                    ImGui::Text("0x%02X", bp.m_value);
                    break;
                case DebuggerState::BreakpointType::InstructionCount:
                    ImGui::Text("%u", bp.m_value);
                    break;
                }
                
                // Enabled column with toggle
                ImGui::TableNextColumn();
                bool enabled = bp.m_enabled;
                if (ImGui::Checkbox(("##Enabled" + std::to_string(bp.m_id)).c_str(), &enabled))
                {
                    ToggleBreakpoint(data, emulator, bp.m_id);
                }
                
                // Action column
                ImGui::TableNextColumn();
                ImGui::PushID(bp.m_id);
                if (ImGui::Button("Remove"))
                {
                    RemoveBreakpoint(data, emulator, bp.m_id);
                    ImGui::PopID();
                    break; // Exit loop since we modified the vector
                }
                ImGui::PopID();
            }
            ImGui::EndTable();
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // --- Memory View ---
    if (ImGui::CollapsingHeader("Memory", ImGuiTreeNodeFlags_DefaultOpen)) 
    {
        constexpr float MemoryViewHeight = 200.0f;

        // Calculate columns for memory view
        float avail = ImGui::GetContentRegionAvail().x;
        float tableWidth = avail - ImGui::GetStyle().ScrollbarSize;
        float cellWidth = ImGui::CalcTextSize("FF").x + ImGui::GetStyle().CellPadding.x * 2;
        int cols = static_cast<int>((tableWidth - 80) / cellWidth);
        cols--;
        if (cols < 1) cols = 1;
        int totalCols = cols + 1;

        // Memory search and navigation
        ImGui::PushItemWidth(120);
        static char searchAddr[5] = "";
        static uint16_t searchTarget = 0;
        if (ImGui::InputText("Search", searchAddr, IM_ARRAYSIZE(searchAddr), ImGuiInputTextFlags_CharsHexadecimal))
        {
            // Convert hex string to uint16_t
            searchTarget = static_cast<uint16_t>(strtoul(searchAddr, nullptr, 16));
        }
        ImGui::SameLine();
        if (ImGui::Button("Go"))
        {
            if (searchTarget < EMULATOR_GB_MEMORY_SIZE)
            {
                m_state.m_selectedMemoryCell = searchTarget;
                // Calculate the row that contains this address
                int row = searchTarget / cols;
                // Calculate scroll position to center the row in the view
                float rowHeight = ImGui::GetTextLineHeightWithSpacing();
                float viewHeight = MemoryViewHeight;
                float scrollPos = (row * rowHeight) - (viewHeight / 2) + (rowHeight / 2);
                // Ensure we don't scroll past the start
                scrollPos = std::max(0.0f, scrollPos);
                m_state.m_memoryScrollTarget = scrollPos;
            }
        }
        ImGui::PopItemWidth();

        ImGui::Spacing();

        // Memory viewer
        uint16_t regionAddr = m_state.m_hoveredAddr >= 0 ? m_state.m_hoveredAddr : m_state.m_memoryFirstVisibleAddr;
        regionAddr = m_state.m_selectedMemoryCell >= 0 ? m_state.m_selectedMemoryCell : regionAddr;

        ImU32 regionColor = GetMemRegionTint(regionAddr);
        const char* regionLabel = GetMemRegionName(regionAddr);

        uint8_t* mem = static_cast<uint8_t*>(data.m_rawMemoryView);

        // Create a fixed-height container for the region label
        ImGui::BeginChild("RegionLabel", ImVec2(0, ImGui::GetTextLineHeightWithSpacing() + ImGui::GetStyle().FramePadding.y * 2), false);
        ImVec2 labelSize = ImGui::CalcTextSize(regionLabel);
        ImVec2 cursor = ImGui::GetCursorScreenPos();
        ImVec2 padding = ImGui::GetStyle().FramePadding;
        ImVec2 labelRectMin = cursor;
        ImVec2 labelRectMax = ImVec2(cursor.x + labelSize.x + padding.x * 2, cursor.y + labelSize.y + padding.y * 2);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddRectFilled(labelRectMin, labelRectMax, regionColor, 4.0f);

        ImGui::SetCursorScreenPos(ImVec2(cursor.x + padding.x, cursor.y + padding.y));
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32_WHITE);
        ImGui::TextUnformatted(regionLabel);
        ImGui::PopStyleColor();

        if (m_state.m_selectedMemoryCell >= 0 || m_state.m_hoveredAddr >= 0)
        {
            ImGui::SameLine();
            int selectedMemory = m_state.m_selectedMemoryCell >= 0 ? m_state.m_selectedMemoryCell : m_state.m_hoveredAddr;
            ImGui::Text("Addr: 0x%04X", selectedMemory);
            ImGui::SameLine(); 
            ImGui::Text("Value: 0x%02X", GetMem(mem, selectedMemory));
            ImGui::SameLine();
            if (ImGui::Button("Add BP", ImVec2(50, 20)))
            {
                AddBreakpoint(data, emulator, DebuggerState::BreakpointType::MemoryWrite, static_cast<uint16_t>(selectedMemory));
            }
        }
        ImGui::EndChild();

        ImGui::BeginChild("MemoryView", ImVec2(0, MemoryViewHeight), false);
        // Set scroll position if we have a target
        if (m_state.m_memoryScrollTarget >= 0)
        {
            ImGui::SetScrollY(m_state.m_memoryScrollTarget);
            m_state.m_memoryScrollTarget = -1; // Reset target
        }

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
                        ImU32 tint = GetMemRegionTint(addr);
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, tint);

                        // Highlight breakpoints with red tint
                        if (HasBreakpointAtAddress(data, static_cast<uint16_t>(addr)))
                        {
                            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(255, 0, 0, 80));
                        }

                        if (addr == m_state.m_selectedMemoryCell)
                        {
                            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(0, 255, 0, 100));
                        }

                        ImGui::PushID(addr);

                        char buf[8];
                        sprintf_s(buf, "%02X", GetMem(mem, addr));

                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

                        if (ImGui::Button(buf, ImVec2(ImGui::GetColumnWidth(), ImGui::GetTextLineHeightWithSpacing()))) 
                        {
                            m_state.m_selectedMemoryCell = (m_state.m_selectedMemoryCell == addr) ? -1 : addr;
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

    ImGui::End();
    ImGui::PopStyleColor(7);
    ImGui::PopStyleVar(6);
}

void DebuggerUI::Toggle(DebuggerState& data)
{
    m_state.m_showWindow = !m_state.m_showWindow;
    data.m_debuggerActive = m_state.m_showWindow;
}
#else
void DebuggerUI::Draw(DebuggerState& data, Emulator* emulator) {}

void DebuggerUI::Toggle(DebuggerState& data) {}
#endif