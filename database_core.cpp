#include "database_core.h"
#include <fstream>
#include <cstring>
#include <cstdio>

using namespace std;

FileDatabase::FileDatabase(string file) : file(file)
{
    rebuild_index();
}

void FileDatabase::rebuild_index()
{
    index_table.clear();
    ifstream f(file, ios::binary);
    if (!f.is_open())
    {
        return;
    }
    Song s;
    while (f.read(reinterpret_cast<char *>(&s), sizeof(Song)))
    {
        streampos pos = f.tellg() - streamoff(sizeof(Song));
        if (!s.is_deleted)
        {
            index_table[s.id] = pos;
        }
    }
}

bool FileDatabase::remove_database()
{
    clear_database();
    if (remove(file.c_str()) == 0)
    {
        return true;
    }
    return false;
}

bool FileDatabase::add_record(const Song &song, string &err)
{
    if (index_table.find(song.id) != index_table.end())
    {
        err = "Ошибка: Песня уже есть в базе!";
        return false;
    }
    ofstream f(file, ios::binary | ios::app);
    if (!f.is_open())
    {
        err = "Ошибка: Не удалось открыть файл для записи!";
        return false;
    }
    f.seekp(0, ios::end);
    streampos pos = f.tellp();
    index_table[song.id] = pos;
    return true;
}

bool FileDatabase::delete_bykey(const string id)
{
    auto it = index_table.find(id);
    if (it == index_table.end())
    {
        return false;
    }
    fstream f(file, ios::binary | ios::in | ios::out);
    if (!f.is_open())
    {
        return false;
    }
    f.seekp(it->second);
    Song s;
    f.read(reinterpret_cast<char *>(&s), sizeof(Song));
    s.is_deleted = true;

    f.seekp(it->second);
    f.write(reinterpret_cast<const char *>(&s), sizeof(Song));
    index_table.erase(it);
    return true;
}

int FileDatabase::delete_byfield(const string &field, const string &value)
{
    fstream f(file, ios::binary | ios::in | ios::out);
    if (!f.is_open())
        return 0;

    Song s;
    int count = 0;
    while (f.read(reinterpret_cast<char *>(&s), sizeof(Song)))
    {
        if (s.is_deleted)
            continue;

        bool match = false;
        if (field == "title" && string(s.title) == value)
            match = true;
        else if (field == "artist" && string(s.artist) == value)
            match = true;

        if (match)
        {
            streampos pos = f.tellg() - streamoff(sizeof(Song));
            s.is_deleted = true;

            f.seekp(pos);
            f.write(reinterpret_cast<const char *>(&s), sizeof(Song));
            index_table.erase(s.id);
            count++;

            f.seekg(pos + streamoff(sizeof(Song)));
        }
    }
    return count;
}

bool FileDatabase::find_bykey(const string &id, Song &result)
{
    auto it = index_table.find(id);
    if (it == index_table.end())
        return false;

    ifstream f(file, ios::binary);
    if (!f.is_open())
        return false;

    f.seekg(it->second);
    f.read(reinterpret_cast<char *>(&result), sizeof(Song));
    return true;
}

vector<Song> FileDatabase::find_byfield(const string &field, const string &value)
{
    vector<Song> results;
    ifstream f(file, ios::binary);
    if (!f.is_open())
        return results;

    Song s;
    while (f.read(reinterpret_cast<char *>(&s), sizeof(Song)))
    {
        if (s.is_deleted)
            continue;

        bool match = false;
        if (field == "title" && string(s.title) == value)
            match = true;
        else if (field == "artist" && string(s.artist) == value)
            match = true;
        else if (field == "year" && to_string(s.release_year) == value)
            match = true;

        if (match)
            results.push_back(s);
    }
    return results;
}

bool FileDatabase::update_record(const Song &updated_song)
{
    auto it = index_table.find(updated_song.id);
    if (it == index_table.end())
        return false;

    fstream f(file, ios::binary | ios::in | ios::out);
    if (!f.is_open())
        return false;

    f.seekp(it->second);
    f.write(reinterpret_cast<const char *>(&updated_song), sizeof(Song));
    return true;
}

bool FileDatabase::create_backup(const string &backup_file)
{
    ifstream src(file, ios::binary);
    if (!src.is_open())
        return false;

    ofstream dst(backup_file, ios::binary | ios::trunc);
    if (!dst.is_open())
        return false;

    dst << src.rdbuf();
    return true;
}

bool FileDatabase::restore_frombackup(const string &backup_file)
{
    ifstream src(backup_file, ios::binary);
    if (!src.is_open())
        return false;

    ofstream dst(file, ios::binary | ios::trunc);
    if (!dst.is_open())
        return false;

    dst << src.rdbuf();

    src.close();
    dst.close();

    rebuild_index();
    return true;
}

bool FileDatabase::import_toxlsx(const string &xlsx_file)
{
    ofstream csv(xlsx_file);
    if (!csv.is_open())
        return false;

    csv << "ISRC;Title;Artist;Release Year;Duration\n";

    vector<Song> active_songs = get_all_valid_records();

    for (const auto &song : active_songs)
    {
        csv << song.id << ";"
            << song.title << ";"
            << song.artist << ";"
            << song.release_year << ";"
            << song.duration << "\n";
    }
    return true;
}

vector<Song> FileDatabase::get_all_valid_records()
{
    vector<Song> records;
    ifstream f(file, ios::binary);
    if (!f.is_open())
    {
        return records;
    }
    Song s;
    while (f.read(reinterpret_cast<char *>(&s), sizeof(Song)))
    {
        if (!s.is_deleted)
        {
            records.push_back(s);
        }
    }
    return records;
}