#include "database_core.h"
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <cstring>
#include <string>
#include <vector>

using namespace std;

static void glfw_error_callback(int error, const char *description) {}

int main(int, char **)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    const char *glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow *window = glfwCreateWindow(1024, 768, "Song Database", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    FileDatabase db("songs_database.dat");

    char in_id[16] = "";
    char in_title[128] = "";
    char in_artist[128] = "";
    int in_year = 2026;
    double in_duration = 3.0;

    char search_val[128] = "";
    int search_type = 0;
    bool is_searching = false;

    vector<Song> current_view;
    string system_msg = "System is ready.";
    bool msg_is_error = false;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (!is_searching)
        {
            current_view = db.get_all_valid_records();
        }

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("Database Control Panel", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        ImGui::Columns(2, "MainColumns", true);
        ImGui::SetColumnWidth(0, 500.0f);

        ImGui::Text("ADD / EDIT RECORD");
        ImGui::Separator();
        ImGui::InputText("ID (Key)", in_id, IM_ARRAYSIZE(in_id));
        ImGui::InputText("Title", in_title, IM_ARRAYSIZE(in_title));
        ImGui::InputText("Artist", in_artist, IM_ARRAYSIZE(in_artist));
        ImGui::InputInt("Release Year", &in_year);
        ImGui::InputDouble("Duration", &in_duration, 0.1, 1.0, "%.2f");

        ImGui::Spacing();
        if (ImGui::Button("Add Song", ImVec2(160, 0)))
        {
            Song s;
            string err;
            strncpy(s.id, in_id, sizeof(s.id) - 1);
            s.id[sizeof(s.id) - 1] = '\0';
            strncpy(s.title, in_title, sizeof(s.title) - 1);
            s.title[sizeof(s.title) - 1] = '\0';
            strncpy(s.artist, in_artist, sizeof(s.artist) - 1);
            s.artist[sizeof(s.artist) - 1] = '\0';
            s.release_year = in_year;
            s.duration = in_duration;
            s.is_deleted = false;

            if (db.add_record(s, err))
            {
                system_msg = "Song successfully added to file.";
                msg_is_error = false;
                is_searching = false;
            }
            else
            {
                system_msg = err;
                msg_is_error = true;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Update by ID", ImVec2(160, 0)))
        {
            Song s;
            strncpy(s.id, in_id, sizeof(s.id) - 1);
            s.id[sizeof(s.id) - 1] = '\0';
            strncpy(s.title, in_title, sizeof(s.title) - 1);
            s.title[sizeof(s.title) - 1] = '\0';
            strncpy(s.artist, in_artist, sizeof(s.artist) - 1);
            s.artist[sizeof(s.artist) - 1] = '\0';
            s.release_year = in_year;
            s.duration = in_duration;
            s.is_deleted = false;

            if (db.update_record(s))
            {
                system_msg = "Song data successfully updated in place.";
                msg_is_error = false;
                is_searching = false;
            }
            else
            {
                system_msg = "Error: Song with this ID not found.";
                msg_is_error = true;
            }
        }

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Text("SEARCH AND DELETE");
        ImGui::Separator();
        ImGui::RadioButton("By ID (Key)", &search_type, 0);
        ImGui::SameLine();
        ImGui::RadioButton("By Title", &search_type, 1);
        ImGui::SameLine();
        ImGui::RadioButton("By Artist", &search_type, 2);
        ImGui::SameLine();
        ImGui::RadioButton("By Year", &search_type, 3);
        ImGui::SameLine();
        ImGui::RadioButton("By Duration", &search_type, 4);

        ImGui::InputText("Value", search_val, IM_ARRAYSIZE(search_val));

        if (ImGui::Button("Search", ImVec2(100, 0)))
        {
            if (strlen(search_val) == 0)
            {
                is_searching = false;
                system_msg = "Showing all records.";
                msg_is_error = false;
            }
            else
            {
                current_view.clear();
                if (search_type == 0)
                {
                    Song res;
                    if (db.find_bykey(search_val, res))
                        current_view.push_back(res);
                }
                else if (search_type == 1)
                {
                    current_view = db.find_byfield("title", search_val);
                }
                else if (search_type == 2)
                {
                    current_view = db.find_byfield("artist", search_val);
                }
                else if (search_type == 3)
                    current_view = db.find_byfield("year", search_val);
                else if (search_type == 4)
                    current_view = db.find_byfield("duration", search_val);

                is_searching = true;

                system_msg = "Search completed. Records found: " + to_string(current_view.size());
                msg_is_error = false;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Delete", ImVec2(100, 0)))
        {
            if (search_type == 0)
            {
                if (db.delete_bykey(search_val))
                {
                    system_msg = "Record successfully marked as deleted.";
                    msg_is_error = false;
                }
                else
                {
                    system_msg = "Error: ID not found.";
                    msg_is_error = true;
                }
            }
            else
            {
                string f_name = "title";
                if (search_type == 1)
                    f_name = "title";
                else if (search_type == 2)
                    f_name = "artist";
                else if (search_type == 3)
                    f_name = "year";
                else if (search_type == 4)
                    f_name = "duration";

                int deleted = db.delete_byfield(f_name, search_val);
                system_msg = "Batch deletion completed. Deleted: " + to_string(deleted);
                msg_is_error = false;
            }
            is_searching = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset", ImVec2(100, 0)))
        {
            is_searching = false;
            search_val[0] = '\0';
            system_msg = "Filters reset.";
            msg_is_error = false;
        }

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Text("SERVICE FUNCTIONS");
        ImGui::Separator();
        if (ImGui::Button("Create Backup", ImVec2(320, 0)))
        {
            if (db.create_backup("backup.bak"))
            {
                system_msg = "Backup file backup.bak successfully created.";
                msg_is_error = false;
            }
            else
            {
                system_msg = "Error creating backup.";
                msg_is_error = true;
            }
        }
        if (ImGui::Button("Restore from Backup", ImVec2(320, 0)))
        {
            if (db.restore_frombackup("backup.bak"))
            {
                system_msg = "Database restored. Hash table rebuilt.";
                msg_is_error = false;
                is_searching = false;
            }
            else
            {
                system_msg = "Error: Backup file not found.";
                msg_is_error = true;
            }
        }
        if (ImGui::Button("Export to Excel (CSV)", ImVec2(320, 0)))
        {
            if (db.import_toxlsx("export.csv"))
            {
                system_msg = "Data successfully exported to export.csv.";
                msg_is_error = false;
            }
            else
            {
                system_msg = "Error exporting text data.";
                msg_is_error = true;
            }
        }
        if (ImGui::Button("Clear Database File", ImVec2(155, 0)))
        {
            db.clear_database();
            system_msg = "File truncated to 0 bytes. RAM table cleared.";
            msg_is_error = false;
            is_searching = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("DESTROY DB", ImVec2(155, 0)))
        {
            if (db.remove_database())
            {
                system_msg = "Working file completely erased from disk.";
                msg_is_error = false;
                is_searching = false;
            }
            else
            {
                system_msg = "Error deleting (file might be in use).";
                msg_is_error = true;
            }
        }

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Text("SYSTEM STATUS:");
        if (msg_is_error)
        {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s", system_msg.c_str());
        }
        else
        {
            ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "%s", system_msg.c_str());
        }

        ImGui::NextColumn();

        ImGui::Text("ACTIVE RECORDS");
        ImGui::Separator();

        if (ImGui::BeginTable("DBTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0, 650)))
        {
            ImGui::TableSetupColumn("ID (Key)");
            ImGui::TableSetupColumn("Song Title");
            ImGui::TableSetupColumn("Artist");
            ImGui::TableSetupColumn("Year");
            ImGui::TableSetupColumn("Duration");
            ImGui::TableHeadersRow();

            for (const auto &song : current_view)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", song.id);
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", song.title);
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%s", song.artist);
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%d", song.release_year);
                ImGui::TableSetColumnIndex(4);
                ImGui::Text("%.2f min", song.duration);
            }
            ImGui::EndTable();
        }

        ImGui::End();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}