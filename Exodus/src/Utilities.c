#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "merc.h"
#include "tables.h"
#include "clans/new_clans.h"
#include "Utilities.h"

bool is_metal(OBJ_DATA *obj) {
    if ((obj->material != NULL) && ((strcmp(obj->material, "silver") == 0) || (strcmp(obj->material, "iron") == 0) ||
        (strcmp(obj->material, "mithril") == 0) || (strcmp(obj->material, "platinum") == 0) ||
        (strcmp(obj->material, "lead") == 0) || (strcmp(obj->material, "copper") == 0) ||
        (strcmp(obj->material, "electrum") == 0) || (strcmp(obj->material, "bronze") == 0) ||
        (strcmp(obj->material, "brass") == 0) || (strcmp(obj->material, "tin") == 0) ||
        (strcmp(obj->material, "pewter") == 0) ||  (strcmp(obj->material, "metal") == 0) ||
        (strcmp(obj->material, "steel") == 0) || (strcmp(obj->material, "gold") == 0) ||
        (strcmp(obj->material, "adamantite") == 0) || (strcmp(obj->material, "aluminum") == 0))) {
            return TRUE;
        }

    return TRUE;
}

bool is_iron(OBJ_DATA *obj) {
    if ((obj->material != NULL) && ((strcmp(obj->material, "iron") == 0) || (strcmp(obj->material, "steel") == 0))) {
        return TRUE;
    }

    return FALSE;
}

bool is_char_in_natural_sector(CHAR_DATA *ch) {
    if ((ch->in_room) && ((ch->in_room->sector_type == SECT_FOREST) ||
            (ch->in_room->sector_type == SECT_HILLS) ||
            (ch->in_room->sector_type == SECT_MOUNTAIN) ||
            (ch->in_room->sector_type == SECT_DESERT) ||            
            (ch->in_room->sector_type == SECT_GRASSLAND) ||
            (ch->in_room->sector_type == SECT_TUNDRA) ||    
            (ch->in_room->sector_type == SECT_SWAMP)))    
                return TRUE;

    return FALSE;
}

bool is_char_in_water_sector(CHAR_DATA *ch) {
    if ((ch->in_room) && ((ch->in_room->sector_type == SECT_WATER_SWIM) ||
            (ch->in_room->sector_type == SECT_WATER_NOSWIM) ||
            (ch->in_room->sector_type == SECT_WATER_OCEAN) ||
            (ch->in_room->sector_type == SECT_UNDERWATER)))
                return TRUE;

    return FALSE;
}

char *escape_color(char *str) {
    char new_str[MAX_STRING_LENGTH];
    char *cptr;
    int tcnt = 0;
    for (cptr = str; *cptr != '\0'; cptr++) {
        if (*cptr == '`') {
            cptr++;
        } else {
            new_str[tcnt] = *cptr;
            tcnt++;
        }

        if (*cptr == '\0')
            break;
    }
    
    strcpy(str, new_str);
    return str;
}

bool is_animal(CHAR_DATA *ch) {
    char *race = race_table[ch->race].name; 
    if ((strcmp(race, "bat") == 0) || (strcmp(race, "bear") == 0) ||
        (strcmp(race, "cat") == 0) || (strcmp(race, "dog") == 0) || (strcmp(race, "fox") == 0) ||
        (strcmp(race, "fido") == 0) || (strcmp(race, "lizard") == 0) || (strcmp(race, "modron") == 0) ||
        (strcmp(race, "pig") == 0) || (strcmp(race, "rabbit") == 0) || (strcmp(race, "snake") == 0) ||
        (strcmp(race, "song bird") == 0) || (strcmp(race, "water fowl") == 0) || (strcmp(race, "wolf") == 0) ||
        (strcmp(race, "wyvern") == 0) || (strcmp(race, "fish") == 0))
            return TRUE;
    
    return FALSE;
}

int find_damage_number_by_noun(char *noun) {
    int i;
    for (i = 0; i <= MAX_WEAPON_FLAGS; i++) {
        if (strcmp(weapon_flags[i].name, noun) == 0)
            return weapon_flags[i].bit;
    }
    
    return 0;
}

bool is_switched_pk_pc(CHAR_DATA *ch) {
    if (is_switched_pc(ch) && (ch->desc->original->pcdata->loner || ch->desc->original->clan != CLAN_BOGUS)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

bool is_switched_pc(CHAR_DATA *ch) {
    if (IS_NPC(ch) && 
        IS_SET(ch->act2, ACT_PUPPET) && 
        (ch->desc && ch->desc->original)) {
            return TRUE;
    } else {
        return FALSE;
    }
}
