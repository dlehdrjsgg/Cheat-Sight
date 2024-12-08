#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

#define MAX_PATH 512
const char *MODULES[] = {"Text", "Diagram", "Image"};

typedef struct {
    char output_dir[MAX_PATH];
    char *selected_file;         // 파일 선택 경로
    GList *selected_modules;     // 선택된 모듈 목록
    GtkWidget *text_view;        // 텍스트 출력 뷰
    GtkWidget *analyze_button;   // 분석 시작 버튼
    GtkWidget *file_button;      // 파일 선택 버튼
    GtkWidget *check_button1;    // 체크박스 1
    GtkWidget *check_button2;    // 체크박스 2
    GtkWidget *check_button3;    // 체크박스 3
} AppData;

// 함수 선언
void append_to_text_view(AppData *app, const char *message);
void open_file_dialog(GtkWidget *widget, gpointer user_data);
void on_analyze_button_clicked(GtkWidget *widget, gpointer user_data);
void on_analysis_start(GtkWidget *widget, gpointer data);
void process_file(AppData *app, const char *file, int option1, int option2, int option3);

// 텍스트 뷰에 메시지 추가
void append_to_text_view(AppData *app, const char *message) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->text_view));
    gtk_text_buffer_insert_at_cursor(buffer, message, -1);
}

// 명령 실행 함수
void execute_command(AppData *app, const char *command) {
    FILE *pipe = popen(command, "r");
    if (!pipe) {
        append_to_text_view(app, "[오류] 명령 실행 실패\n");
        return;
    }

    char output[512];
    while (fgets(output, sizeof(output), pipe)) {
        append_to_text_view(app, output);
    }

    int status = pclose(pipe);
    if (status != 0) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "[오류] 명령 실행 중 상태 코드 %d 반환\n", WEXITSTATUS(status));
        append_to_text_view(app, error_msg);
    }
}

// 파일 선택 함수
void open_file_dialog(GtkWidget *widget, gpointer user_data) {
    AppData *app = (AppData *)user_data;
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "파일 선택", GTK_WINDOW(gtk_widget_get_toplevel(widget)),
        GTK_FILE_CHOOSER_ACTION_OPEN, "_취소", GTK_RESPONSE_CANCEL, "_열기", GTK_RESPONSE_ACCEPT, NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        if (app->selected_file) {
            g_free(app->selected_file);
        }
        app->selected_file = gtk_file_chooser_get_filename(chooser);

        char message[512];
        snprintf(message, sizeof(message), "분석 파일: %s\n", app->selected_file);
        append_to_text_view(app, message);

        gtk_widget_set_sensitive(app->analyze_button, TRUE);
        gtk_widget_set_sensitive(app->file_button, FALSE);
    }
    gtk_widget_destroy(dialog);
}

// 모드 창 함수
void on_analyze_button_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *option_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(option_window), "모드 선택");
    gtk_window_set_default_size(GTK_WINDOW(option_window), 400, 300);
    gtk_window_set_position(GTK_WINDOW(option_window), GTK_WIN_POS_CENTER);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(option_window), vbox);

    GtkWidget *label = gtk_label_new("분석을 시작하려면 아래 모드를 선택하세요.");
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

    check_button1 = gtk_check_button_new_with_label("프로그램 요약");
    gtk_box_pack_start(GTK_BOX(vbox), check_button1, FALSE, FALSE, 0);
    check_button2 = gtk_check_button_new_with_label("다이어그램 생성");
    gtk_box_pack_start(GTK_BOX(vbox), check_button2, FALSE, FALSE, 0);
    check_button3 = gtk_check_button_new_with_label("이미지 생성");
    gtk_box_pack_start(GTK_BOX(vbox), check_button3, FALSE, FALSE, 0);

    GtkWidget *start_button = gtk_button_new_with_label("분석 시작");
    gtk_box_pack_end(GTK_BOX(vbox), start_button, FALSE, FALSE, 0); 
    g_signal_connect(start_button, "clicked", G_CALLBACK(on_analysis_start), 
                     g_strdup_printf("%s", selected_file));  

    g_signal_connect(start_button, "clicked", G_CALLBACK(gtk_widget_destroy), option_window);
    gtk_widget_show_all(option_window); 
}

void on_analysis_start(GtkWidget *widget, gpointer data) {
    char *file = (char *)data;
    int option1 = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button1)) ? 1 : 0;
    int option2 = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button2)) ? 1 : 0;
    int option3 = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button3)) ? 1 : 0;

    printf("[LOG] option1 = %d, option2 = %d, option3 = %d\n", option1, option2, option3);

    char message[512];
    snprintf(message, sizeof(message), "모드 정보: 프로그램 요약 (%d), 다이어그램 생성 (%d), 이미지 생성 (%d)\n", option1, option2, option3);
    append_to_text_view(message);
    process_file(file, option1, option2, option3);
}

void on_download_button_clicked(GtkWidget *widget, gpointer user_data) {
    const char *file_name = (const char *)user_data;
    char source_path[512];
    char destination_path[512];

    snprintf(source_path, sizeof(source_path), "%s/%s", output_dir, file_name);
    snprintf(destination_path, sizeof(destination_path), "../../Downloads/%s", file_name);

    GFile *source_file = g_file_new_for_path(source_path);
    GFile *destination_file = g_file_new_for_path(destination_path);
    GError *error = NULL;
    if (g_file_copy(source_file, destination_file, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, &error)) {
        printf("[LOG] '%s' downloaded successfully to '%s'\n", file_name, destination_path);
    } else {
        printf("[ERROR] Failed to copy '%s': %s\n", file_name, error->message);
        g_error_free(error);
    }

    g_object_unref(source_file);
    g_object_unref(destination_file);
}

void on_result_view_button_clicked() {
    struct dirent *entry;
    DIR *dir = opendir(output_dir);
    printf("[LOG] result button clicked\n");

    if (dir == NULL) {
        perror("[ERROR] Failed to open directory");
        return;
    }

    GtkWidget *result_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(result_window), "Cheat-Sight 결과");
    gtk_window_set_default_size(GTK_WINDOW(result_window), 600, 400);
    gtk_window_set_position(GTK_WINDOW(result_window), GTK_WIN_POS_CENTER);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(result_window), vbox);

    GtkWidget *top_label = gtk_label_new("모든 분석이 종료되었습니다. 각 분석 결과를 확인하려면 다운로드 버튼을 클릭하세요.\n\t\t\t\t팀 너무답답해요, 2024.12.3");
    gtk_box_pack_start(GTK_BOX(vbox), top_label, FALSE, FALSE, 10);  

    GtkWidget *file_list_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), file_list_box, TRUE, TRUE, 10);  

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
        gtk_box_pack_start(GTK_BOX(file_list_box), hbox, TRUE, TRUE, 5);

        GtkWidget *label = gtk_label_new(entry->d_name);
        gtk_widget_set_halign(label, GTK_ALIGN_CENTER); 
        gtk_widget_set_valign(label, GTK_ALIGN_CENTER); 
        gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 5);

        GtkWidget *download_button = gtk_button_new_with_label("Download");
        gtk_widget_set_halign(download_button, GTK_ALIGN_CENTER); 
        gtk_widget_set_valign(download_button, GTK_ALIGN_CENTER); 
        gtk_box_pack_start(GTK_BOX(hbox), download_button, TRUE, TRUE, 5);

        g_signal_connect(download_button, "clicked", G_CALLBACK(on_download_button_clicked), g_strdup(entry->d_name));
    }

    closedir(dir);
    gtk_widget_show_all(result_window);
}

void process_file(const char *file, int option1, int option2, int option3) {
    char message[512];
    printf("[LOG] module process run\n");
    snprintf(message, sizeof(message), "\n[INFO] 모든 선택이 정상적으로 끝나 분석을 시작합니다.\n");
    append_to_text_view(message); 

    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(output_dir, sizeof(output_dir), "output/%Y-%m-%d-%H-%M-%S", tm_info);

    printf("[LOG] output file create : %s\n", output_dir);
    snprintf(message, sizeof(message), "[INFO] 출력 폴더 생성: %s\n", output_dir);
    append_to_text_view(message); 
    char mkdir_command[512];
    snprintf(mkdir_command, sizeof(mkdir_command), "mkdir -p %s", output_dir);
    system(mkdir_command);  

    // Python 스크립트 실행
    if (option1) {
        snprintf(message, sizeof(message), "\n[INFO] 모듈 불러오는 중 (Text)...\n");
        append_to_text_view(message);

        char command[512];
        snprintf(command, sizeof(command), "python3 -u modules/Text.py %s %s", file, output_dir); 

        FILE *fp = popen(command, "r");
        if (fp == NULL) {
            snprintf(message, sizeof(message), "[ERROR] Python 스크립트를 실행할 수 없습니다.\n");
            append_to_text_view(message);
        } else {
            char buffer[512];
            while (fgets(buffer, sizeof(buffer), fp) != NULL) {
                append_to_text_view(buffer);
            }
            fclose(fp);
        }
    } else if (option2) {
        snprintf(message, sizeof(message), "\n[INFO] 모듈 불러오는 중 (Diagram)...\n");
        append_to_text_view(message);

        char command[512];
        snprintf(command, sizeof(command), "python3 -u modules/Diagram.py %s %s", file, output_dir);  

        FILE *fp = popen(command, "r");
        if (fp == NULL) {
            snprintf(message, sizeof(message), "[ERROR] Python 스크립트를 실행할 수 없습니다.\n");
            append_to_text_view(message);
        } else {
            char buffer[512];
            while (fgets(buffer, sizeof(buffer), fp) != NULL) {
                append_to_text_view(buffer);
            }
            fclose(fp);
        }
    } else if (option3) {
        snprintf(message, sizeof(message), "\n[INFO] 모듈 불러오는 중 (Image)...\n");
        append_to_text_view(message);

        char command[512];
        snprintf(command, sizeof(command), "python3 -u modules/Image.py %s %s", file, output_dir);  

        FILE *fp = popen(command, "r");
        if (fp == NULL) {
            snprintf(message, sizeof(message), "[ERROR] Python 스크립트를 실행할 수 없습니다.\n");
            append_to_text_view(message);
        } else {
            char buffer[512];
            while (fgets(buffer, sizeof(buffer), fp) != NULL) {
                append_to_text_view(buffer);
            }
            fclose(fp);
        }
    }else {
        snprintf(message, sizeof(message), "\n[ERROR] 해당 모듈이 설정되어 있지 않습니다.\n");
        append_to_text_view(message); 
    }

    printf("[LOG] process clear!\n");
    snprintf(message, sizeof(message), "\n\n[INFO] 분석 종료\n");
    append_to_text_view(message); 

    gtk_button_set_label(GTK_BUTTON(analyze_button), "결과 보기");
    gtk_widget_set_sensitive(analyze_button, TRUE);

    g_signal_connect(analyze_button, "clicked", G_CALLBACK(on_result_view_button_clicked), NULL);
}

void on_cancel_button_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *window = GTK_WIDGET(data);
    gtk_widget_show_all(window); 
    GtkWidget *second_window = gtk_widget_get_toplevel(widget);
    gtk_widget_destroy(second_window); 
}

void on_start_button_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *window = GTK_WIDGET(data); 
    GtkWidget *second_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(second_window), "Cheat-Sight 분석");
    gtk_window_set_default_size(GTK_WINDOW(second_window), 600, 400);

    gtk_window_set_position(GTK_WINDOW(second_window), GTK_WIN_POS_CENTER); 

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(second_window), vbox);

    GtkWidget *label = gtk_label_new("프로그램 동작과정을 분석하고 시각화할 코드를 선택하세요.");
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);

    file_button = gtk_button_new_with_label("파일 선택");
    gtk_box_pack_start(GTK_BOX(vbox), file_button, FALSE, FALSE, 5);
    g_signal_connect(file_button, "clicked", G_CALLBACK(open_file_dialog), second_window);

    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE); 
    gtk_box_pack_start(GTK_BOX(vbox), text_view, TRUE, TRUE, 5);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);

    analyze_button = gtk_button_new_with_label("모드 선택");
    gtk_widget_set_size_request(analyze_button, 200, 50); 
    gtk_box_pack_start(GTK_BOX(hbox), analyze_button, TRUE, TRUE, 5);
    g_signal_connect(analyze_button, "clicked", G_CALLBACK(on_analyze_button_clicked), NULL);
    gtk_widget_set_sensitive(analyze_button, FALSE);  

    GtkWidget *cancel_button = gtk_button_new_with_label("취소");
    gtk_widget_set_size_request(cancel_button, 200, 50); 
    gtk_box_pack_start(GTK_BOX(hbox), cancel_button, TRUE, TRUE, 5);
    g_signal_connect(cancel_button, "clicked", G_CALLBACK(on_cancel_button_clicked), window);

    g_object_set_data(G_OBJECT(window), "second_window", second_window);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_hide(window); 
    gtk_widget_show_all(second_window); 
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Cheat-Sight");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER); 

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget *label1 = gtk_label_new("Cheat-Sight");
    GtkWidget *label2 = gtk_label_new("팀 너무답답해요");

    GtkCssProvider *provider1 = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider1,
                                    "label { font-family: Sans; font-size: 70pt; }\n", -1, NULL);
    gtk_style_context_add_provider(gtk_widget_get_style_context(label1), GTK_STYLE_PROVIDER(provider1), GTK_STYLE_PROVIDER_PRIORITY_USER);

    GtkCssProvider *provider2 = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider2,
                                    "label { font-family: Sans; font-size: 20pt; }\n", -1, NULL);
    gtk_style_context_add_provider(gtk_widget_get_style_context(label2), GTK_STYLE_PROVIDER(provider2), GTK_STYLE_PROVIDER_PRIORITY_USER);

    GtkWidget *text_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(text_box), label1, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(text_box), label2, FALSE, FALSE, 10);

    gtk_widget_set_halign(text_box, GTK_ALIGN_CENTER); 
    gtk_widget_set_valign(text_box, GTK_ALIGN_CENTER);  
    gtk_box_pack_start(GTK_BOX(vbox), text_box, TRUE, TRUE, 0);

    GtkWidget *start_button = gtk_button_new_with_label("프로그램 시작");
    gtk_widget_set_halign(start_button, GTK_ALIGN_CENTER);  
    gtk_box_pack_end(GTK_BOX(vbox), start_button, FALSE, FALSE, 20); 
    g_signal_connect(start_button, "clicked", G_CALLBACK(on_start_button_clicked), window);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
