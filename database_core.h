#pragma once
#include "song.h"
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

class FileDatabase
{
private:
    string file;
    unordered_map<string, streampos> index_table;

    void rebuild_index();

public:
    FileDatabase(string db_file = "database.dat");
    bool remove_database();
    void clear_database();
    bool add_record(const Song &song, string &err);
    bool delete_bykey(const string id);
    int delete_byfield(const string &field, const string &value);
    bool find_bykey(const string &id, Song &result);
    vector<Song> find_byfield(const string &field, const string &value);
    bool update_record(const Song &updated_song);
    bool create_backup(const string &backup_file);
    bool restore_frombackup(const string &backup_file);
    bool import_from_txt(const string &txt_file);
    bool import_toxlsx(const string &xlsx_file);

    vector<Song> get_all_valid_records();
};
