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

    #define PLAYER(t,name)         (*sv_actor_stat_##name[t])
    #define PLAYERSTR(a,t,attr)    defformatstring(a)("sv_%s%s", playertypes[t][4], #attr)
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
    #define PLAYER(t,name)         (*actor_stat_##name[t])
    #define PLAYERSTR(a,t,attr)    defformatstring(a)("%s%s", playertypes[t][4], #attr)
#endif
