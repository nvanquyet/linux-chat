//
// Created by vawnwuyest on 4/1/25.
//
#ifndef GROUP_H
#define GROUP_H

#include <user.h>
#define MAX_MEMBERS 100
#define MAX_GROUPS 100

typedef struct Group {
    int id;
    char name[50];
    long created_at;
    User *created_by;
} Group;
extern Group groups[MAX_GROUPS];
extern int group_count;
void addGroup(Group *group);
void groupRefresh();
void deleteGroup(Group *group);
void listGroupMembers(int index);
void showJoinedGroups();
Group *findGroup(int index);

void handle_joined_groups();

#endif //GROUP_H
