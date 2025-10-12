// #include "VideoLayer.h"

// #include <imgui.h>
// #include <imgui_internal.h>
// #include <imgui_stdlib.h>  
// #include <glad/glad.h>

// using namespace FrameKit;
// using namespace FrameKit::MediaKit;

// namespace SandBox {
//     static GLenum kUploadFmt = GL_RGBA;

//     VideoLayer::VideoLayer(std::string name) : Layer(std::move(name)) {}

//     void VideoLayer::OnAttach() {
//         FK_PROFILE_FUNCTION();
        
//         m_FileDlg.filters = {"mp4","mkv","avi","mov","webm","ts","m2ts","mpg","mpeg","flv","wmv","ogv","m4v","wav","mp3","aac","flac","ogg"};

//         m_Alive.store(true, std::memory_order_release);
//         m_Path.clear();        
//     }

//     void VideoLayer::OnDetach() {
//         FK_PROFILE_FUNCTION();
//         m_Alive.store(false, std::memory_order_release);
//         if (m_Player) {
//             m_Player->setVideoSink(nullptr);   // <-- prevent callbacks
//             m_Player->close();
//             m_Player.reset();
//         }
//         if (m_Tex) { DestroyTexture(m_Tex); m_Tex = 0; }
//         m_HasPending.store(false);
//         m_PendingRGBA.clear();
//     }

//     void VideoLayer::OnAsyncUpdate() { FK_PROFILE_FUNCTION(); }

//     void VideoLayer::OnSyncUpdate(Timestep) {}

//     void VideoLayer::OnEvent(Event&) {}

//     void VideoLayer::OnRender() {
//         FK_PROFILE_FUNCTION();

//         ImGui::Begin("VideoPort", nullptr,
//             ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

//         // Path field + Open button
//         ImGui::SetNextItemWidth(420.0f);
//         if (ImGui::InputTextWithHint("##media_path",
//                                     "Enter media path or drop a file here",
//                                     &m_Path,
//                                     ImGuiInputTextFlags_EnterReturnsTrue)) {
//             if (!m_Path.empty()) OpenMedia(m_Path);
//         }
//         ImGui::SameLine();
//         if (ImGui::Button("Open")) {
//             if (!m_Path.empty()) OpenMedia(m_Path);
//         }
//         ImGui::SameLine();

//         if (ImGui::Button("Browse")) {
//             m_FileDlg.open = true;
//             ImGui::OpenPopup("Open Media");
//         }
//         std::string picked;
//         if (m_FileDlg.Show("Open Media", picked)) {
//             m_Path = picked;
//             OpenMedia(m_Path);
//         }

//         // Drag-and-drop file support (from OS/file manager)
//         if (ImGui::BeginDragDropTarget()) {
//             if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATH")) {
//                 // If you use ImGui’s backends that expose OS file drops as text:
//                 const char* p = (const char*)payload->Data;
//                 if (p && *p) { m_Path = p; OpenMedia(m_Path); }
//             }
//             // Fallback: many backends deliver a plain text path
//             if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXT")) {
//                 const char* p = (const char*)payload->Data;
//                 if (p && *p) { m_Path = p; OpenMedia(m_Path); }
//             }
//             ImGui::EndDragDropTarget();
//         }

//         // If no player yet, stop here after the UI
//         if (!m_Player) {
//             ImGui::TextUnformatted("No media opened");
//             ImGui::End();
//             return;
//         }

//         // --- existing controls and rendering below ---
//         const auto  st  = m_Player->state();
//         const double now = m_Player->time();
//         const double dur = m_Player->info().demux.durationSec;

//         if (ImGui::Button(m_Paused ? "Play" : "Pause")) {
//             if (m_Paused) m_Player->play(); else m_Player->pause();
//             m_Paused = !m_Paused;
//         }
//         ImGui::SameLine();
//         if (ImGui::Button("Stop")) { m_Player->stop(); m_Paused = true; }
//         ImGui::SameLine();
//         if (ImGui::Button("Seek 0")) { m_Player->seek(0.0); }

//         ImGui::SameLine();
//         ImGui::SetNextItemWidth(120.0f);
//         float rate_f = static_cast<float>(m_Rate);
//         if (ImGui::SliderFloat("Rate", &rate_f, 0.10f, 5.00f, "%.2fx")) {
//             m_Rate = static_cast<double>(rate_f);
//             m_Player->setRate(m_Rate);
//         }
//         if (ImGui::Checkbox("Loop", &m_Loop)) m_Player->setLoop(m_Loop);

//         if (dur > 0.0) {
//             double t = now, tmin = 0.0, tmax = dur;
//             ImGui::SetNextItemWidth(280.0f);
//             if (ImGui::SliderScalar("Seek", ImGuiDataType_Double, &t, &tmin, &tmax, "%.2f s")) {
//                 if (std::fabs(t - now) > 0.0005) m_Player->seek(t, false);
//             }
//         }
//         ImGui::Text("State: %s | t %.3f s / %.3f s", ToString(st), now, dur);

//         // upload pending frame if any
//         if (m_HasPending.exchange(false, std::memory_order_acq_rel)) {
//             std::vector<uint8_t> rgba; int w = 0, h = 0;
//             { std::lock_guard<std::mutex> lk(m_FrameMtx);
//             rgba = std::move(m_PendingRGBA); w = m_PendingW; h = m_PendingH; }
//             EnsureTexture(w, h);
//             kUploadFmt = GL_RGBA;
//             UpdateTextureRGBA8(m_Tex, rgba.data(), w, h);
//         }

//         if (m_Tex) {
//             ImVec2 avail = ImGui::GetContentRegionAvail();
//             float imgAspect = m_TexH ? float(m_TexW)/float(m_TexH) : 1.0f;
//             float availAspect = avail.y ? avail.x/avail.y : 1.0f;
//             ImVec2 draw = avail;
//             if (imgAspect > availAspect) draw.y = draw.x / imgAspect;
//             else                         draw.x = draw.y * imgAspect;
//             ImVec2 cur = ImGui::GetCursorPos();
//             ImGui::SetCursorPos(ImVec2(cur.x + (avail.x - draw.x)*0.5f,
//                                     cur.y + (avail.y - draw.y)*0.5f));
//             ImGui::Image((ImTextureID)(intptr_t)m_Tex, draw, ImVec2(0,0), ImVec2(1,1));
//         } else {
//             ImGui::TextUnformatted("No frame yet");
//         }

//         ImGui::End();
//     }

//     void VideoLayer::OpenMedia(const std::string& path) {
//         if (m_Player) {
//             m_Player->setVideoSink(nullptr);
//             m_Player->close();
//             m_Player.reset();
//         }
//         m_Tex ? DestroyTexture(m_Tex) : void();
//         m_Tex = 0; m_TexW = m_TexH = 0;
//         m_HasPending.store(false); m_PendingRGBA.clear();

//         PlayerConfig cfg{};
//         cfg.hwDecode = false;
//         cfg.outFmt   = PixelFormat::RGBA8;
//         cfg.deliverGPU = false;
//         cfg.videoQueue = 8;
//         cfg.audioQueue = 32;

//         m_Player = CreatePlayer(PlayerBackend::FFmpeg);
//         if (!m_Player || !m_Player->open(path.c_str(), cfg)) {
//             FK_CORE_ERROR("Open failed: {}", path);
//             m_Player.reset();
//             return;
//         }
//         m_Player->setLoop(m_Loop);
//         m_Player->setRate(m_Rate);
//         m_Player->setVideoSink([this](const MediaKit::VideoFrame& f){
//             if (!m_Alive.load(std::memory_order_acquire) || f.planes.empty()) return;
//             std::lock_guard<std::mutex> lk(m_FrameMtx);
//             m_PendingW = f.info.w; m_PendingH = f.info.h;
//             m_PendingRGBA = f.planes[0];
//             m_HasPending.store(true, std::memory_order_release);
//         });
//         m_Player->play(); m_Paused = false;
//     }
//     // ---- helpers ---------------------------------------------------------------
//     void VideoLayer::EnsureTexture(int w, int h) {
//         if (m_Tex && w == m_TexW && h == m_TexH) return;
//         if (m_Tex) DestroyTexture(m_Tex);
//         m_Tex = CreateTextureRGBA8(w, h);
//         m_TexW = w; m_TexH = h;
//     }

//     uint64_t VideoLayer::CreateTextureRGBA8(int w, int h) {
//         GLuint tex = 0;
//         glGenTextures(1, &tex);
//         glBindTexture(GL_TEXTURE_2D, tex);
//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//         glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//         glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, kUploadFmt, GL_UNSIGNED_BYTE, nullptr);
//         return static_cast<uint64_t>(tex);
//     }

//     void VideoLayer::UpdateTextureRGBA8(uint64_t tex, const void* data, int w, int h) {
//         glBindTexture(GL_TEXTURE_2D, (GLuint)tex);
//         glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//         glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, kUploadFmt, GL_UNSIGNED_BYTE, data);
//     }

//     void VideoLayer::DestroyTexture(uint64_t handle) {
//         GLuint tex = (GLuint)handle;
//         if (tex) glDeleteTextures(1, &tex);
//     }

//     const char* VideoLayer::ToString(PlayerState s) {
//         switch (s) {
//         case PlayerState::Idle:    return "Idle";
//         case PlayerState::Opening: return "Opening";
//         case PlayerState::Paused:  return "Paused";
//         case PlayerState::Playing: return "Playing";
//         case PlayerState::Stopped: return "Stopped";
//         case PlayerState::Ended:   return "Ended";
//         case PlayerState::Error:   return "Error";
//         default:                   return "?";
//         }
//     }
// } // namespace SandBox
