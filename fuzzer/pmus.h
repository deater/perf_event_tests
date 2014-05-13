
struct generic_event_type {
        char *name;
        char *value;
        long long config;
        long long config1;
        long long config2;
};

struct format_type {
        char *name;
        char *value;
        int field;
        unsigned long long mask;
};

struct pmu_type {
        char *name;
        int type;
        int num_formats;
        int num_generic_events;
        struct format_type *formats;
        struct generic_event_type *generic_events;
};

extern int num_pmus;
extern struct pmu_type *pmus;

