#define PATH_SIZE 1024

typedef struct {	
	int sock;	
	char str[PATH_SIZE];
	pthread_mutex_t *mtx_s;
} data;


typedef struct {
        data **queue;
        int start;
        int end;
        int count;
} pool_t;


int flag;
int queue_size;
pthread_mutex_t mtx;
pthread_cond_t cond_nonempty;
pthread_cond_t cond_nonfull;
pool_t pool;


void initialize(pool_t * pool, int x);
void place(pool_t * pool, data * d1);
data * obtain(pool_t * pool);
void * producer(void * ptr);
void * consumer(void * ptr);

