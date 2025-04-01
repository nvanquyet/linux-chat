#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "group.h"

Group groups[MAX_GROUPS];
int group_count = 0;


void addGroup(Group *group) {
    groups[group_count++] = *group;
}

void groupRefresh() {
    group_count = 0;
    memset(groups, 0, sizeof(groups));
    printf("All groups have been reset!\n");
}

void deleteGroup(Group *group) {
    if (group == NULL) {
        printf("Invalid group!\n");
        return;
    }

    for (int i = 0; i < group_count; i++) {
        if (groups[i].id == group->id) {
            printf("Leaving group: %s (ID: %d)\n", groups[i].name, groups[i].id);

            for (int j = i; j < group_count - 1; j++) {
                groups[j] = groups[j + 1];
            }

            group_count--;

            printf("Successfully left group!\n");
            return;
        }
    }

    printf("Group not found!\n");
}


Group *findGroup(int index) {
    if (index >= 0 && index < group_count) {
        return &groups[index];
    }
    return NULL;
}

void listGroupMembers(int index) {
    if (index < 0 || index >= group_count) {
        printf("Invalid group index!\n");
        return;
    }

    Group *group = &groups[index];
    printf("Members of group '%s':\n", group->name);
}

void showJoinedGroups() {
    // printf("Groups joined by %s:\n", user->username);
    //
    // for (int i = 0; i < group_count; i++) {
    //     for (int j = 0; j < groups[i].user_count; j++) {
    //         if (groups[i].member[j]->id == user->id) {
    //             printf("- %s\n", groups[i].name);
    //         }
    //     }
    // }
}
