#include "types.h"

#include <sys/stat.h>

#include <filesystem>

#include <string>
#include <cstring>

#include <cstdio>

// #include <boost/filesystem.hpp>
// #include <boost/dll/runtime_symbol_info.hpp>
// #include <boost/interprocess/file_mapping.hpp>
// #include <boost/interprocess/mapped_region.hpp>

#include "game_code.h"

b32 g_Running = true;
u32 g_FullscreenMode = 0;

// namespace fs = boost::filesystem;
// namespace bip = boost::interprocess;

struct ReplayBuffer {
    std::string FilePath;
    // bip::file_mapping FileMapping;
    // bip::mapped_region MappedRegion;
    void *MemoryBlock;
};

struct ProgramState {
    void *GamePermamentMemoryBlock;
    size_t GamePermanentMemorySize;

    /*
    ReplayBuffer ReplayBuffers[4];
    std::FILE *RecordingHandle = 0;
    int InputRecordingSlot = -1; // the index of the buffer in ReplayBuffers, -1 for nothing recording
    std::FILE *PlaybackHandle = 0;
    int InputPlayingSlot = -1; // the index of the buffer in ReplayBuffers, -1 for nothing playing
    */
    // The game loop uses 2 input states ("old" and "new") to handle controls.
    GameInput Input[2] = {};
    // The input state needs to be saved each time we start and restored when we
    // stop playback to avoid a bug where if you stop playback while playing a time
    // when a button was held down, the button continues to be pressed.
    GameInput SavedInputBeforePlay[2];

    // fs::path ExeDir;
};

/*
static std::string get_replay_input_file_location(ProgramState *state, b32 inputStream, int slot) {
    using namespace std::string_literals;

    std::string fileName = "input_recording_"s + std::to_string(slot) + "_"s + (inputStream ? "input" : "state");
    return (state->ExeDir / (fileName)).string();
}
static GameCode load_game_code(fs::path &sourceDLL, fs::path &tempDLL, fs::path &lockFile) {
    GameCode result = {};

    result.LastWriteTime = fs::last_write_time(sourceDLL);
    // Reload only if the compilation is finished (our lock file is deleted)
    if (!fs::exists(lockFile)) {
        fs::copy_file(sourceDLL, tempDLL, fs::copy_option::overwrite_if_exists);

        result.Object = SDL_LoadObject(tempDLL.string().c_str());
        if (result.Object) {
            result.ProcessSDLEvent = (game_process_sdl_event_func *) SDL_LoadFunction(result.Object, "game_process_sdl_event");
            result.UpdateAndRender = (game_update_and_render_func *) SDL_LoadFunction(result.Object, "game_update_and_render");
            result.IsValid         = result.UpdateAndRender;
        }

        if (!result.IsValid) {
            result.UpdateAndRender = 0;
        }
    }
    return result;
}

static void unload_game_code(GameCode *gameCode) {
    if (gameCode->Object) {
        SDL_UnloadObject(gameCode->Object);
        gameCode->Object = 0;
    }

    gameCode->IsValid         = false;
    gameCode->ProcessSDLEvent = 0;
    gameCode->UpdateAndRender = 0;
}
*/

static void process_keyboard_event(GameButtonState *newState, s32 isDown) {
    if (newState->EndedDown != isDown) {
        newState->EndedDown = isDown;
        ++(newState->HalfTransitionCount);
    }
}

/*
static void begin_recording_input(ProgramState *state, int recordingSlot) {
    ReplayBuffer *buffer = &state->ReplayBuffers[recordingSlot];
    if (buffer->MemoryBlock) {
        state->InputRecordingSlot = recordingSlot;

        std::string filePath = get_replay_input_file_location(state, true, recordingSlot);
        fopen_s(&state->RecordingHandle, filePath.c_str(), "w");

        memcpy(buffer->MemoryBlock, state->GamePermamentMemoryBlock, state->GamePermanentMemorySize);
    }
}

static void end_recording_input(ProgramState *state) {
    fclose(state->RecordingHandle);
    state->RecordingHandle = 0;
    state->InputRecordingSlot = -1;
}

static void begin_input_playback(ProgramState *state, int playingSlot) {
    ReplayBuffer *buffer = &state->ReplayBuffers[playingSlot];
    if (buffer->MemoryBlock) {
        state->InputPlayingSlot = playingSlot;

        std::string filePath = get_replay_input_file_location(state, true, playingSlot);
        fopen_s(&state->PlaybackHandle, filePath.c_str(), "r");

        memcpy(state->GamePermamentMemoryBlock, buffer->MemoryBlock, state->GamePermanentMemorySize);
        // Save the input before starting playback. See declaration of SavedInputBeforePlay for explanation.
        memcpy(state->SavedInputBeforePlay, state->Input, sizeof(state->Input) /*warning this sizeof may fail);
    }
}

static void end_input_playback(ProgramState *state) {
    // Restore the state of the input to what it was before start of playback.
    // See declaration of SavedInputBeforePlay for explanation.
    memcpy(state->Input, state->SavedInputBeforePlay, sizeof(state->SavedInputBeforePlay));

    fclose(state->PlaybackHandle);
    state->PlaybackHandle = 0;
    state->InputPlayingSlot = -1;
}

// Read a single frame of input from the file and loop if the end is reached
static void play_input_back(ProgramState *state, GameInput *input) {
    size_t readObjects = fread(input, sizeof(GameInput), 1, state->PlaybackHandle);
    if (readObjects == 0) {
        // Loop back to the beginning when we hit the end.
        int inputPlayingSlot = state->InputPlayingSlot; // save our slot
        end_input_playback(state);
        begin_input_playback(state, inputPlayingSlot);

        fread(input, sizeof(GameInput), 1, state->PlaybackHandle);
    }
}
*/

#ifdef _MSC_VER
// :CRT
void WinMainCRTStartup() {
#else
int main(int argc, char *argv[]) {
#endif
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        exit_program(-1);
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_Window *window = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    SDL_GLContext context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, context);
    SDL_GL_SetSwapInterval(1);

    GameMemory gameMemory;
    gameMemory.SDLWindow = window;
    gameMemory.Permanent.init(1_MB, FreeListAllocator::FIND_BEST);
    gameMemory.Temporary.init(1_MB);

    ProgramState state;
    // state.ExeDir = boost::dll::program_location().parent_path();
    state.GamePermamentMemoryBlock = gameMemory.Permanent.Memory;
    state.GamePermanentMemorySize = gameMemory.Permanent.TotalSize;

    /*
    fs::path gameSourceDLL		= state.ExeDir / "sandbox.dll";
    fs::path gameSourceTempDLL  = state.ExeDir / "sandbox.temp.dll";
    fs::path gameSourceLockfile = state.ExeDir / "lockfile";
    for (int i = 0; i < ArrayCount(state.ReplayBuffers); ++i) {
        ReplayBuffer *buffer = &state.ReplayBuffers[i];

        buffer->FilePath = get_replay_input_file_location(&state, false, i);
        if (!fs::exists(buffer->FilePath)) {
            std::ofstream ofs(buffer->FilePath, std::ios::binary | std::ios::out);
            ofs.seekp(state.GamePermanentMemorySize - 1);
            ofs.write("", 1);
        }

        buffer->FileMapping  = bip::file_mapping(buffer->FilePath.c_str(), bip::mode_t::read_write);

        buffer->MappedRegion = bip::mapped_region(buffer->FileMapping, bip::read_write, 0, state.GamePermanentMemorySize);
        buffer->MemoryBlock  = buffer->MappedRegion.get_address();

        assert(buffer->MemoryBlock);
    }
    */

    GameInput *newInput = &state.Input[0];
    GameInput *oldInput = &state.Input[1];

    // GameCode game = load_game_code(gameSourceDLL, gameSourceTempDLL, gameSourceLockfile);

    s32 gameCodeReloadedTitleFrames = 0, recordingDotTimer = 0;
    b32 recordingDot = true;
    while (g_Running) {
        // Check for game code .dll change by comparing the dates, if it is - reload it
        /*
        std::time_t newWriteTime = fs::last_write_time(gameSourceDLL);
        if (game.LastWriteTime != newWriteTime) {
            unload_game_code(&game);
            while (!(game = load_game_code(gameSourceDLL, gameSourceTempDLL, gameSourceLockfile)).IsValid);
            gameCodeReloadedTitleFrames = 90; // 1.5 seconds with 60f/s
        }
        */
        for (int i = 0; i < ArrayCount(newInput->Buttons); ++i) {
            newInput->Buttons[i].EndedDown = oldInput->Buttons[i].EndedDown;
        }

        /*
        if (state.InputRecordingSlot != -1) {
            fwrite(newInput, sizeof(GameInput), 1, state.RecordingHandle); // Write the input to the file
        }
        if (state.InputPlayingSlot != -1) {
            play_input_back(&state, newInput); // Read the input from the file
        }

        if (game.UpdateAndRender) {
            game.UpdateAndRender(&gameMemory, newInput);
        }
        */
        GameInput *temp = newInput;
        newInput = oldInput;
        oldInput = temp;

        /*
        using namespace std::string_literals;

        std::string titleStateDisplay = "";
        if (gameCodeReloadedTitleFrames-- > 0) {
            titleStateDisplay = "**Game code reloaded**"s;
        } else if (state.InputRecordingSlot != -1) {
            recordingDotTimer++;
            if (recordingDotTimer > 60) {
                recordingDot = !recordingDot;
                recordingDotTimer = 0;
            }
            titleStateDisplay = (recordingDot ? "⬤ Recording on slot "s : "   Recording on slot "s) + std::to_string(state.InputRecordingSlot + 1);
        } else if (state.InputPlayingSlot != -1) {
            recordingDot = true;
            recordingDotTimer = 0;

            titleStateDisplay = "▶ Playing on slot " + std::to_string(state.InputPlayingSlot + 1);
        } else {
            recordingDot = true;
            recordingDotTimer = 0;
        }

        std::string title = "Sandbox"s + (!titleStateDisplay.empty() ? " | "s : ""s) + titleStateDisplay;
        SDL_SetWindowTitle(window, title.c_str());
        */
        SDL_GL_SwapWindow(window);

        // Process events

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                exit_program(0);
            } else if (event.type == SDL_MOUSEMOTION) {
                newInput->MouseX = event.motion.x;
                newInput->MouseY = event.motion.y;
            } else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
                process_keyboard_event(&newInput->MouseButtons[event.button.button], event.button.state);
            } else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                SDL_Keysym keysum = event.key.keysym;

                b32 isDown = event.key.state;
                b32 wasDown = event.key.repeat;

                if (isDown) {
                    // Close on ALF+F4
                    if (keysum.mod & KMOD_ALT && keysum.sym == SDLK_F4)
                        exit_program(0);

                    // Switch fullscreen mode
                    if (keysum.sym == SDLK_F11) {
                        switch (g_FullscreenMode) {
                        case 0:
                            g_FullscreenMode = SDL_WINDOW_FULLSCREEN_DESKTOP;
                            break;
                        case SDL_WINDOW_FULLSCREEN_DESKTOP:
                            g_FullscreenMode = SDL_WINDOW_FULLSCREEN;
                            break;
                        case SDL_WINDOW_FULLSCREEN:
                            g_FullscreenMode = 0;
                            break;
                        }
                        SDL_SetWindowFullscreen(window, g_FullscreenMode);
                    }
                }

                if (wasDown != isDown) {
                    if (keysum.sym == 'w') {
                        process_keyboard_event(&newInput->MoveForward, isDown);
                    } else if (keysum.sym == 'a') {
                        process_keyboard_event(&newInput->MoveLeft, isDown);
                    } else if (keysum.sym == 's') {
                        process_keyboard_event(&newInput->MoveBackward, isDown);
                    } else if (keysum.sym == 'd') {
                        process_keyboard_event(&newInput->MoveRight, isDown);
                    } else if (keysum.sym == SDLK_LSHIFT) {
                        process_keyboard_event(&newInput->Sprint, isDown);
                    } else if (keysum.sym == SDLK_SPACE) {
                        process_keyboard_event(&newInput->Sprint, isDown);

                    } /*else if (isDown && keysum.sym > '0' && (keysum.sym < ('1' + ArrayCount(state.ReplayBuffers)))) {
                        // We support no more than 9 replay buffers.
                        int slot = keysum.sym - '0' - 1;

                        int currentPlayingSlot   = state.InputPlayingSlot;
                        int currentRecordingSlot = state.InputRecordingSlot;

                        // Alt + [Number]: Begin/stop recording
                        // [Number]:	   Begin/stop playback
                        if (keysum.mod & KMOD_ALT) {
                            if (currentRecordingSlot != -1) {
                                SDL_Log("Stopped recording on slot %d.\n", slot);
                                end_recording_input(&state);
                            }
                            if (currentPlayingSlot != -1) {
                                SDL_Log("Stopped playing on slot %d.\n", currentPlayingSlot);
                                end_input_playback(&state);
                            }
                            if (slot != currentRecordingSlot) {
                                SDL_Log("Beginning recording on slot %d.\n", slot);
                                begin_recording_input(&state, slot);
                            }
                        } else {
                            if (currentRecordingSlot == -1) {
                                if (currentPlayingSlot != -1) {
                                    SDL_Log("Stopped playing on slot %d.\n", currentPlayingSlot);
                                    end_input_playback(&state);
                                }
                                if (currentPlayingSlot != slot) {
                                    SDL_Log("Beginning playback on slot %d.\n", slot);
                                    begin_input_playback(&state, slot);
                                }
                            }
                        }

                    }
                    */
                }

                process_keyboard_event(&newInput->MouseButtons[event.button.button], event.key.state);
            }
            // game.ProcessSDLEvent(&gameMemory, newInput, &event);
        }
    }

    // SDL_GL_DeleteContext(gl_context);
    // SDL_DestroyWindow(window);
    // SDL_Quit();
    exit_program(0);
}
