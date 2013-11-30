#include <uwsgi.h>

extern struct uwsgi_server uwsgi;

/*

This is an high-performance memory area shared by all workers/cores/threads

Contrary to the caching subsystem it is 1-copy (caching for non-c apps is 2-copy)

Languages not allowing that kind of access should emulate it calling uwsgi_malloc and then copying it back to
the language object.

The memory areas could be monitored for changes (read: cores can be suspended while waiting for values)

You can configure multiple areas specifying multiple --sharedarea options

This is a very low-level api, try to use it to build higher-level primitives or rely on the caching subsystem

*/

struct uwsgi_sharedarea *uwsgi_sharedarea_get_by_id(int id, uint64_t pos) {
	if (id > uwsgi.sharedareas_cnt-1) return NULL;
	struct uwsgi_sharedarea *sa = uwsgi.sharedareas[id];
	if (pos > sa->max_pos) return NULL;
	return sa;
}

int64_t uwsgi_sharedarea_read(int id, uint64_t pos, char *blob, uint64_t len) {
	struct uwsgi_sharedarea *sa = uwsgi_sharedarea_get_by_id(id, pos);
        if (!sa) return -1;
        if (pos + len > sa->max_pos + 1) return -1;
	if (len == 0) len = sa->honour_used ? sa->used-pos : (sa->max_pos + 1) - pos;
        uwsgi_rlock(sa->lock);
        memcpy(blob, sa->area + pos, len);
        sa->hits++;
        uwsgi_rwunlock(sa->lock);
        return len;
} 

int uwsgi_sharedarea_write(int id, uint64_t pos, char *blob, uint64_t len) {
	struct uwsgi_sharedarea *sa = uwsgi_sharedarea_get_by_id(id, pos);
	if (!sa) return -1;
	if (pos + len > sa->max_pos + 1) return -1;
	uwsgi_wlock(sa->lock);
	memcpy(sa->area + pos, blob, len);	
	sa->updates++;
	uwsgi_rwunlock(sa->lock);
	return 0;
} 

int uwsgi_sharedarea_read64(int id, uint64_t pos, int64_t *value) {
	return uwsgi_sharedarea_read(id, pos, (char *) value, 8);
}

int uwsgi_sharedarea_write64(int id, uint64_t pos, int64_t *value) {
	return uwsgi_sharedarea_write(id, pos, (char *) value, 8);
}

int uwsgi_sharedarea_read8(int id, uint64_t pos, int8_t *value) {
	return uwsgi_sharedarea_read(id, pos, (char *) value, 1);
}

int uwsgi_sharedarea_write8(int id, uint64_t pos, int8_t *value) {
	return uwsgi_sharedarea_write(id, pos, (char *) value, 1);
}

int uwsgi_sharedarea_read16(int id, uint64_t pos, int16_t *value) {
	return uwsgi_sharedarea_read(id, pos, (char *) value, 2);
}

int uwsgi_sharedarea_write16(int id, uint64_t pos, int16_t *value) {
	return uwsgi_sharedarea_write(id, pos, (char *) value, 2);
}


int uwsgi_sharedarea_read32(int id, uint64_t pos, int32_t *value) {
	return uwsgi_sharedarea_read(id, pos, (char *) value, 4);
}

int uwsgi_sharedarea_write32(int id, uint64_t pos, int32_t *value) {
	return uwsgi_sharedarea_write(id, pos, (char *) value, 4);
}

int uwsgi_sharedarea_inc8(int id, uint64_t pos, int8_t amount) {
	struct uwsgi_sharedarea *sa = uwsgi_sharedarea_get_by_id(id, pos);
        if (!sa) return -1;
        if (pos + 1 > sa->max_pos + 1) return -1;
        uwsgi_wlock(sa->lock);
	int8_t *n_ptr = (int8_t *) (sa->area + pos);
        *n_ptr+=amount;
        sa->updates++;
        uwsgi_rwunlock(sa->lock);
        return 0;
}

int uwsgi_sharedarea_inc16(int id, uint64_t pos, int16_t amount) {
        struct uwsgi_sharedarea *sa = uwsgi_sharedarea_get_by_id(id, pos);
        if (!sa) return -1;
        if (pos + 2 > sa->max_pos + 1) return -1;
        uwsgi_wlock(sa->lock);
        int16_t *n_ptr = (int16_t *) (sa->area + pos);
        *n_ptr+=amount;
        sa->updates++;
        uwsgi_rwunlock(sa->lock);
        return 0;
}

int uwsgi_sharedarea_inc32(int id, uint64_t pos, int32_t amount) {
        struct uwsgi_sharedarea *sa = uwsgi_sharedarea_get_by_id(id, pos);
        if (!sa) return -1;
        if (pos + 4 > sa->max_pos + 1) return -1;
        uwsgi_wlock(sa->lock);
        int32_t *n_ptr = (int32_t *) (sa->area + pos);
        *n_ptr+=amount;
        sa->updates++;
        uwsgi_rwunlock(sa->lock);
        return 0;
}

int uwsgi_sharedarea_inc64(int id, uint64_t pos, int64_t amount) {
        struct uwsgi_sharedarea *sa = uwsgi_sharedarea_get_by_id(id, pos);
        if (!sa) return -1;
        if (pos + 4 > sa->max_pos + 1) return -1;
        uwsgi_wlock(sa->lock);
        int64_t *n_ptr = (int64_t *) (sa->area + pos);
        *n_ptr+=amount;
        sa->updates++;
        uwsgi_rwunlock(sa->lock);
        return 0;
}


int uwsgi_sharedarea_dec8(int id, uint64_t pos, int8_t amount) {
        struct uwsgi_sharedarea *sa = uwsgi_sharedarea_get_by_id(id, pos);
        if (!sa) return -1;
        if (pos + 1 > sa->max_pos + 1) return -1;
        uwsgi_wlock(sa->lock);
        int8_t *n_ptr = (int8_t *) (sa->area + pos);
        *n_ptr-=amount;
        sa->updates++;
        uwsgi_rwunlock(sa->lock);
        return 0;
}

int uwsgi_sharedarea_dec16(int id, uint64_t pos, int16_t amount) {
        struct uwsgi_sharedarea *sa = uwsgi_sharedarea_get_by_id(id, pos);
        if (!sa) return -1;
        if (pos + 2 > sa->max_pos + 1) return -1;
        uwsgi_wlock(sa->lock);
        int16_t *n_ptr = (int16_t *) (sa->area + pos);
        *n_ptr-=amount;
        sa->updates++;
        uwsgi_rwunlock(sa->lock);
        return 0;
}

int uwsgi_sharedarea_dec32(int id, uint64_t pos, int32_t amount) {
        struct uwsgi_sharedarea *sa = uwsgi_sharedarea_get_by_id(id, pos);
        if (!sa) return -1;
        if (pos + 4 > sa->max_pos + 1) return -1;
        uwsgi_wlock(sa->lock);
        int32_t *n_ptr = (int32_t *) (sa->area + pos);
        *n_ptr-=amount;
        sa->updates++;
        uwsgi_rwunlock(sa->lock);
        return 0;
}

int uwsgi_sharedarea_dec64(int id, uint64_t pos, int64_t amount) {
        struct uwsgi_sharedarea *sa = uwsgi_sharedarea_get_by_id(id, pos);
        if (!sa) return -1;
        if (pos + 4 > sa->max_pos + 1) return -1;
        uwsgi_wlock(sa->lock);
        int64_t *n_ptr = (int64_t *) (sa->area + pos);
        *n_ptr-=amount;
        sa->updates++;
        uwsgi_rwunlock(sa->lock);
        return 0;
}



/*
	returns:
		0 -> on updates
		-1 -> on error
		-2 -> on timeout
*/
int uwsgi_sharedarea_wait(int id, int freq, int timeout) {
	int waiting = 0;
	struct uwsgi_sharedarea *sa = uwsgi_sharedarea_get_by_id(id, 0);
	if (!sa) return -1;
	if (!freq) freq = 100;
	uwsgi_rlock(sa->lock);
	uint64_t updates = sa->updates;
	uwsgi_rwunlock(sa->lock);
	while(timeout == 0 || (timeout > 0 && (waiting/1000) >= timeout)) {
		uwsgi.wait_milliseconds_hook(freq);
		waiting += freq;
		// lock sa
		uwsgi_rlock(sa->lock);
		if (sa->updates != updates) {
			uwsgi_rwunlock(sa->lock);
			return 0;
		}
		// unlock sa
		uwsgi_rwunlock(sa->lock);
	}
	return -2;
}

int uwsgi_sharedarea_new_id() {
	int id = uwsgi.sharedareas_cnt;
        uwsgi.sharedareas_cnt++;
        if (!uwsgi.sharedareas) {
                uwsgi.sharedareas = uwsgi_malloc(sizeof(struct uwsgi_sharedarea *));
        }
        else {
                struct uwsgi_sharedarea **usa = realloc(uwsgi.sharedareas, ((sizeof(struct uwsgi_sharedarea *)) * uwsgi.sharedareas_cnt));
                if (!usa) {
                        uwsgi_error("uwsgi_sharedarea_init()/realloc()");
                        exit(1);
                }
                uwsgi.sharedareas = usa;
        }
	return id;
}

static struct uwsgi_sharedarea *announce_sa(struct uwsgi_sharedarea *sa) {
	uwsgi_log("sharedarea %d created at %p (%d pages, area at %p)\n", sa->id, sa, sa->pages, sa->area);
	return sa;
}

struct uwsgi_sharedarea *uwsgi_sharedarea_init(int pages) {
	int id = uwsgi_sharedarea_new_id();
	uwsgi.sharedareas[id] = uwsgi_calloc_shared(uwsgi.page_size * (pages + 1));
	uwsgi.sharedareas[id]->area = ((char *) uwsgi.sharedareas[id]) + uwsgi.page_size;
	uwsgi.sharedareas[id]->id = id;
	uwsgi.sharedareas[id]->fd = -1;
	uwsgi.sharedareas[id]->pages = pages;
	uwsgi.sharedareas[id]->max_pos = (uwsgi.page_size * pages) -1;
	char *id_str = uwsgi_num2str(id);
	uwsgi.sharedareas[id]->lock = uwsgi_rwlock_init(uwsgi_concat2("sharedarea", id_str));
	free(id_str);
	return announce_sa(uwsgi.sharedareas[id]);
}

struct uwsgi_sharedarea *uwsgi_sharedarea_init_ptr(char *area, uint64_t len) {
        int id = uwsgi_sharedarea_new_id();
        uwsgi.sharedareas[id] = uwsgi_calloc_shared(sizeof(struct uwsgi_sharedarea));
        uwsgi.sharedareas[id]->area = area;
        uwsgi.sharedareas[id]->id = id;
        uwsgi.sharedareas[id]->fd = -1;
        uwsgi.sharedareas[id]->pages = len / uwsgi.page_size;
	if (len % uwsgi.page_size != 0) uwsgi.sharedareas[id]->pages++;
        uwsgi.sharedareas[id]->max_pos = len-1;
        char *id_str = uwsgi_num2str(id);
        uwsgi.sharedareas[id]->lock = uwsgi_rwlock_init(uwsgi_concat2("sharedarea", id_str));
        free(id_str);
	return announce_sa(uwsgi.sharedareas[id]);
}

struct uwsgi_sharedarea *uwsgi_sharedarea_init_keyval(char *arg) {
	char *s_pages = NULL;
	char *s_file = NULL;
	char *s_fd = NULL;
	char *s_ptr = NULL;
	char *s_size = NULL;
	if (uwsgi_kvlist_parse(arg, strlen(arg), ',', '=',
		"pages", &s_pages,
		"file", &s_file,
		"fd", &s_fd,
		"ptr", &s_ptr,
		"size", &s_size,
		NULL)) {
	}

	return NULL;
}


void uwsgi_sharedareas_init() {
	struct uwsgi_string_list *usl = NULL;
	uwsgi_foreach(usl, uwsgi.sharedareas_list) {
		char *is_keyval = strchr(usl->value, '=');
		if (!is_keyval) {
			uwsgi_sharedarea_init(atoi(usl->value));
		}
		else {
			uwsgi_sharedarea_init_keyval(usl->value);
		}
	}
}
