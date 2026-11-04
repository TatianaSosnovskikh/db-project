#pragma once

struct Song
{
    char id[16];
    char title[128];
    char artist[128];
    int release_year;
    double duration;
    bool is_deleted;
};