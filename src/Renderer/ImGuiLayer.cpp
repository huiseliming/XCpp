#include "ImGuiLayer.h"
#include "Renderer.h"
#include "Core/ScriptManager.h"

#include <backends/imgui_impl_sdl2.h>

void CImGuiLayer::DrawFrame() {
  ImGuiIO IO = ImGui::GetIO();
  DrawDockSpace();
  // Our state
  bool show_demo_window = true;
  bool show_another_window = false;

  ImGui::Begin("decl_tree");  // Create a window called "Hello, world!" and append into it.
  struct SDeclTreeItem : public std::enable_shared_from_this<SDeclTreeItem> {
    RDecl* Decl{nullptr};
    std::weak_ptr<SDeclTreeItem> Parent;
    std::vector<std::shared_ptr<SDeclTreeItem>> Children;
    void AddChild(std::shared_ptr<SDeclTreeItem>& child) {
      child->Parent = weak_from_this();
      Children.push_back(child);
    }
  };
  std::shared_ptr<SDeclTreeItem> decl_root = std::make_shared<SDeclTreeItem>();
  std::unordered_map<RDecl*, std::shared_ptr<SDeclTreeItem>> item_map;
  for (auto& [decl_name, decl] : g_decl_manager.GetDeclMap()) {
    auto it = item_map.find(decl);
    if (it == item_map.end()) {
      std::shared_ptr<SDeclTreeItem> item_it = std::make_shared<SDeclTreeItem>();
      item_it->Decl = decl;
      item_map.insert(std::make_pair(decl, item_it));
      RDecl* decl_it = decl;
      while (decl_it->GetOwner()) {
        auto owner_it = item_map.find(decl_it->GetOwner());
        if (owner_it != item_map.end()) {
          owner_it->second->AddChild(item_it);
          break;
        }
        std::shared_ptr<SDeclTreeItem> owner_item = std::make_shared<SDeclTreeItem>();
        owner_item->Decl = decl_it->GetOwner();
        owner_item->AddChild(item_it);
        item_map.insert(std::make_pair(decl_it->GetOwner(), owner_item));
        decl_it = decl_it->GetOwner();
        item_it = owner_item;
      }
      if (decl_it->GetOwner() == nullptr) {
        decl_root->AddChild(item_it);
      }
    }
  }
  struct CRecursivelyDeclTreeItem {
    void operator()(std::shared_ptr<SDeclTreeItem>& item) {
      if (ImGui::TreeNode(item->Decl->GetName().c_str())) {
        for (auto& child_item : item->Children) {
          operator()(child_item);
        }
        ImGui::TreePop();
      }
    }
  } RecursivelyDeclTreeNode;
  for (auto& child_item : decl_root->Children) {
    RecursivelyDeclTreeNode(child_item);
  }
  //// returns the node's rectangle
  // ImRect RenderTree(Node * n) {
  //   const bool recurse = ImGui::TreeNode(...);
  //   const ImRect nodeRect = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());

  //  if (recurse) {
  //    const ImColor TreeLineColor = ImGui::GetColorU32(ImGuiCol_Text);
  //    const float SmallOffsetX = 11.0f;  // for now, a hardcoded value; should take into account tree indent size
  //    ImDrawList* drawList = ImGui::GetWindowDrawList();

  //    ImVec2 verticalLineStart = ImGui::GetCursorScreenPos();
  //    verticalLineStart.x += SmallOffsetX;  // to nicely line up with the arrow symbol
  //    ImVec2 verticalLineEnd = verticalLineStart;

  //    for (Node* child : *n) {
  //      const float HorizontalTreeLineSize = 8.0f;  // chosen arbitrarily
  //      const ImRect childRect = RenderTree(child);
  //      const float midpoint = (childRect.Min.y + childRect.Max.y) / 2.0f;
  //      drawList->AddLine(ImVec2(verticalLineStart.x, midpoint), ImVec(verticalLineStart.x + HorizontalTreeLineSize,
  //      midpoint),
  //                        TreeLineColor);
  //      verticalLineEnd.y = midpoint;
  //    }

  //    drawList->AddLine(verticalLineStart, verticalLineEnd, TreeLineColor);
  //  }

  //  return nodeRect;
  //}
  ImGui::End();

  // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more
  // about Dear ImGui!).
  if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

  // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
  {
    static float f = 0.0f;
    static int counter = 0;

    ImGui::Begin("Hello, world!");  // Create a window called "Hello, world!" and append into it.

    ImGui::Text("This is some useful text.");           // Display some text (you can use a format strings too)
    ImGui::Checkbox("Demo Window", &show_demo_window);  // Edit bools storing our window open/close state
    ImGui::Checkbox("Another Window", &show_another_window);

    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);  // Edit 1 float using a slider from 0.0f to 1.0f
    ImGui::ColorEdit3("clear color", (float*)&Renderer->ClearColor);  // Edit 3 floats representing a color

    if (ImGui::Button("Button"))  // Buttons return true when clicked (most widgets return true when edited/activated)
      counter++;
    ImGui::SameLine();
    ImGui::Text("counter = %d", counter);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / IO.Framerate, IO.Framerate);
    ImGui::End();
  }

  // 3. Show another simple window.
  if (show_another_window) {
    ImGui::Begin("Another Window", &show_another_window);  // Pass a pointer to our bool variable (the window will have a
                                                           // closing button that will clear the bool when clicked)
    ImGui::Text("Hello from another window!");
    if (ImGui::Button("Close Me")) show_another_window = false;
    ImGui::End();
  }
}

void CImGuiLayer::ProcessEvent(SDL_Event* Event) {
  ImGui_ImplSDL2_ProcessEvent(Event);
}

void CImGuiLayer::HelpMarker(const char* desc) {
  ImGui::TextDisabled("(?)");
  if (ImGui::BeginItemTooltip()) {
    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
    ImGui::TextUnformatted(desc);
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
  }
}

void CImGuiLayer::DrawDockSpace() {  // READ THIS !!!
  // TL;DR; this demo is more complicated than what most users you would normally use.
  // If we remove all options we are showcasing, this demo would become:
  //     void ShowExampleAppDockSpace()
  //     {
  //         ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
  //     }
  // In most cases you should be able to just call DockSpaceOverViewport() and ignore all the code below!
  // In this specific demo, we are not using DockSpaceOverViewport() because:
  // - (1) we allow the host window to be floating/moveable instead of filling the viewport (when opt_fullscreen == false)
  // - (2) we allow the host window to have padding (when opt_padding == true)
  // - (3) we expose many flags and need a way to have them visible.
  // - (4) we have a local menu bar in the host window (vs. you could use BeginMainMenuBar() + DockSpaceOverViewport()
  //      in your code, but we don't here because we allow the window to be floating)

  static bool opt_fullscreen = true;
  static bool opt_padding = false;
  static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

  // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
  // because it would be confusing to have two docking targets within each others.
  ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
  if (opt_fullscreen) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                    ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
  } else {
    dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
  }
  dockspace_flags |= ImGuiDockNodeFlags_PassthruCentralNode;
  // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
  // and handle the pass-thru hole, so we ask Begin() to not render a background.
  if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) window_flags |= ImGuiWindowFlags_NoBackground;

  // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
  // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
  // all active windows docked into it will lose their parent and become undocked.
  // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
  // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
  if (!opt_padding) ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("DockSpace Demo", &bRenderDockSpace, window_flags);
  if (!opt_padding) ImGui::PopStyleVar();

  if (opt_fullscreen) ImGui::PopStyleVar(2);

  // Submit the DockSpace
  ImGuiIO& io = ImGui::GetIO();
  if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
  } else {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("ERROR: Docking is not enabled! See Demo > Configuration.");
    ImGui::Text("Set io.ConfigFlags |= ImGuiConfigFlags_DockingEnable in your code, or ");
    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::SmallButton("click here")) io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  }

  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("Options")) {
      // Disabling fullscreen would allow the window to be moved to the front of other windows,
      // which we can't undo at the moment without finer window depth/z control.
      ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
      ImGui::MenuItem("Padding", NULL, &opt_padding);
      ImGui::Separator();

      if (ImGui::MenuItem("Flag: NoSplit", "", (dockspace_flags & ImGuiDockNodeFlags_NoSplit) != 0)) {
        dockspace_flags ^= ImGuiDockNodeFlags_NoSplit;
      }
      if (ImGui::MenuItem("Flag: NoResize", "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0)) {
        dockspace_flags ^= ImGuiDockNodeFlags_NoResize;
      }
      if (ImGui::MenuItem("Flag: NoDockingInCentralNode", "",
                          (dockspace_flags & ImGuiDockNodeFlags_NoDockingInCentralNode) != 0)) {
        dockspace_flags ^= ImGuiDockNodeFlags_NoDockingInCentralNode;
      }
      if (ImGui::MenuItem("Flag: AutoHideTabBar", "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0)) {
        dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar;
      }
      if (ImGui::MenuItem("Flag: PassthruCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0,
                          opt_fullscreen)) {
        dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode;
      }
      ImGui::Separator();

      if (ImGui::MenuItem("Close", NULL, false)) bRenderDockSpace = false;
      ImGui::EndMenu();
    }
    HelpMarker(
        "When docking is enabled, you can ALWAYS dock MOST window into another! Try it now!"
        "\n"
        "- Drag from window title bar or their tab to dock/undock."
        "\n"
        "- Drag from window menu button (upper-left button) to undock an entire node (all windows)."
        "\n"
        "- Hold SHIFT to disable docking (if io.ConfigDockingWithShift == false, default)"
        "\n"
        "- Hold SHIFT to enable docking (if io.ConfigDockingWithShift == true)"
        "\n"
        "This demo app has nothing to do with enabling docking!"
        "\n\n"
        "This demo app only demonstrate the use of ImGui::DockSpace() which allows you to manually create a docking node "
        "_within_ another window."
        "\n\n"
        "Read comments in ShowExampleAppDockSpace() for more details.");

    ImGui::EndMenuBar();
  }

  ImGui::End();
}
