#ifdef MEK
#ifdef GAMESERVER
    #define PPVAR(flags, name, mn, mx, w00, w01, w02, w03) \
        GVAR(flags, mek1##name, mn, w00, mx); \
        GVAR(flags, mek2##name, mn, w01, mx); \
        GVAR(flags, mek3##name, mn, w02, mx); \
        GVAR(flags, mek4##name, mn, w03, mx); \
        int *sv_actor_stat_##name[] = { \
            &sv_mek1##name, \
            &sv_mek2##name, \
            &sv_mek3##name, \
            &sv_mek4##name \
        };

    #define PPFVAR(flags, name, mn, mx, w00, w01, w02, w03) \
        GFVAR(flags, mek1##name, mn, w00, mx); \
        GFVAR(flags, mek2##name, mn, w01, mx); \
        GFVAR(flags, mek3##name, mn, w02, mx); \
        GFVAR(flags, mek4##name, mn, w03, mx); \
        float *sv_actor_stat_##name[] = { \
            &sv_mek1##name, \
            &sv_mek2##name, \
            &sv_mek3##name, \
            &sv_mek4##name \
        };

    #define PPSVAR(flags, name, w00, w01, w02, w03) \
        GSVAR(flags, mek1##name, w00); \
        GSVAR(flags, mek2##name, w01); \
        GSVAR(flags, mek3##name, w02); \
        GSVAR(flags, mek4##name, w03); \
        char **sv_actor_stat_##name[] = { \
            &sv_mek1##name, \
            &sv_mek2##name, \
            &sv_mek3##name, \
            &sv_mek4##name \
        };

    #define PLAYER(actor,name)         (*sv_actor_stat_##name[actor])
    #define PLAYERSTR(a,actor,attr)    defformatstring(a)("sv_%s%s", playertypes[actor][4], #attr)
#else
#ifdef GAMEWORLD
    #define PPVAR(flags, name, mn, mx, w00, w01, w02, w03) \
        GVAR(flags, mek1##name, mn, w00, mx); \
        GVAR(flags, mek2##name, mn, w01, mx); \
        GVAR(flags, mek3##name, mn, w02, mx); \
        GVAR(flags, mek4##name, mn, w03, mx); \
        int *actor_stat_##name[] = { \
            &mek1##name, \
            &mek2##name, \
            &mek3##name, \
            &mek4##name \
        };

    #define PPFVAR(flags, name, mn, mx, w00, w01, w02, w03) \
        GFVAR(flags, mek1##name, mn, w00, mx); \
        GFVAR(flags, mek2##name, mn, w01, mx); \
        GFVAR(flags, mek3##name, mn, w02, mx); \
        GFVAR(flags, mek4##name, mn, w03, mx); \
        float *actor_stat_##name[] = { \
            &mek1##name, \
            &mek2##name, \
            &mek3##name, \
            &mek4##name \
        };

    #define PPSVAR(flags, name, w00, w01, w02, w03) \
        GSVAR(flags, mek1##name, w00); \
        GSVAR(flags, mek2##name, w01); \
        GSVAR(flags, mek3##name, w02); \
        GSVAR(flags, mek4##name, w03); \
        char **actor_stat_##name[] = { \
            &mek1##name, \
            &mek2##name, \
            &mek3##name, \
            &mek4##name \
        };
#else
    #define PPVAR(flags, name, mn, mx, w00, w01, w02, w03) \
        GVAR(flags, mek1##name, mn, w00, mx); \
        GVAR(flags, mek2##name, mn, w01, mx); \
        GVAR(flags, mek3##name, mn, w02, mx); \
        GVAR(flags, mek4##name, mn, w03, mx); \
        extern int *actor_stat_##name[];
    #define PPFVAR(flags, name, mn, mx, w00, w01, w02, w03) \
        GFVAR(flags, mek1##name, mn, w00, mx); \
        GFVAR(flags, mek2##name, mn, w01, mx); \
        GFVAR(flags, mek3##name, mn, w02, mx); \
        GFVAR(flags, mek4##name, mn, w03, mx); \
        extern float *actor_stat_##name[];
    #define PPSVAR(flags, name, w00, w01, w02, w03) \
        GSVAR(flags, mek1##name, w00); \
        GSVAR(flags, mek2##name, w01); \
        GSVAR(flags, mek3##name, w02); \
        GSVAR(flags, mek4##name, w03); \
        extern char **actor_stat_##name[];
#endif
    #define PLAYER(actor,name)         (*actor_stat_##name[actor])
    #define PLAYERSTR(a,actor,attr)    defformatstring(a)("%s%s", playertypes[actor][4], #attr)
#endif
#else // FPS
#ifdef GAMESERVER
    #define PPVAR(flags, name, mn, mx, w00) \
        GVAR(flags, actor##name, mn, w00, mx); \
        int *sv_actor_stat_##name[] = { \
            &sv_actor##name, \
            &sv_actor##name \
        };

    #define PPFVAR(flags, name, mn, mx, w00) \
        GFVAR(flags, actor##name, mn, w00, mx); \
        float *sv_actor_stat_##name[] = { \
            &sv_actor##name, \
            &sv_actor##name \
        };

    #define PPSVAR(flags, name, w00) \
        GSVAR(flags, actor##name, w00); \
        char **sv_actor_stat_##name[] = { \
            &sv_actor##name, \
            &sv_actor##name \
        };

    #define PLAYER(actor,name)         (*sv_actor_stat_##name[actor])
    #define PLAYERSTR(a,actor,attr)    defformatstring(a)("sv_%s%s", playertypes[actor][4], #attr)
#else
#ifdef GAMEWORLD
    #define PPVAR(flags, name, mn, mx, w00) \
        GVAR(flags, actor##name, mn, w00, mx); \
        int *actor_stat_##name[] = { \
            &actor##name, \
            &actor##name \
        };

    #define PPFVAR(flags, name, mn, mx, w00) \
        GFVAR(flags, actor##name, mn, w00, mx); \
        float *actor_stat_##name[] = { \
            &actor##name, \
            &actor##name \
        };

    #define PPSVAR(flags, name, w00) \
        GSVAR(flags, actor##name, w00); \
        char **actor_stat_##name[] = { \
            &actor##name, \
            &actor##name \
        };
#else
    #define PPVAR(flags, name, mn, mx, w00) \
        GVAR(flags, actor##name, mn, w00, mx); \
        extern int *actor_stat_##name[];
    #define PPFVAR(flags, name, mn, mx, w00) \
        GFVAR(flags, actor##name, mn, w00, mx); \
        extern float *actor_stat_##name[];
    #define PPSVAR(flags, name, w00) \
        GSVAR(flags, actor##name, w00); \
        extern char **actor_stat_##name[];
#endif
    #define PLAYER(actor,name)         (*actor_stat_##name[actor])
    #define PLAYERSTR(a,actor,attr)    defformatstring(a)("%s%s", playertypes[actor][4], #attr)
#endif
#endif
