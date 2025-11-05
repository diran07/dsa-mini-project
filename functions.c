#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* =======================================================================
   ASSET INVENTORY (Array/List)
   ======================================================================= */
#define ASSET_MAX 200
#define ASSET_NAME_LEN 64
#define ASSET_STATUS_LEN 16

typedef struct {
    int asset_id;
    char name[ASSET_NAME_LEN];
    char status[ASSET_STATUS_LEN]; /* "Active" or "Inactive" */
} Asset;

static Asset g_assets[ASSET_MAX];
static int g_asset_count = 0;

static void asset_display_one(const Asset *a) {
    printf("Asset ID: %d | Name: %s | Status: %s\n", a->asset_id, a->name, a->status);
}

static void asset_display_all(void) {
    printf("\n--- Current Asset Inventory ---\n");
    if (g_asset_count == 0) {
        printf("(empty)\n");
        return;
    }
    printf("%-10s %-20s %-10s\n", "Asset ID", "Name", "Status");
    for (int i = 0; i < g_asset_count; ++i) {
        printf("%-10d %-20s %-10s\n", g_assets[i].asset_id, g_assets[i].name, g_assets[i].status);
    }
}

static int asset_search_index(int id) {
    for (int i = 0; i < g_asset_count; ++i) {
        if (g_assets[i].asset_id == id) return i;
    }
    return -1;
}

static void asset_add(int id, const char *name, const char *status) {
    if (g_asset_count >= ASSET_MAX) {
        printf("Inventory full! Cannot add more assets.\n");
        return;
    }
    if (asset_search_index(id) >= 0) {
        printf("Asset with ID %d already exists. Use update instead.\n", id);
        return;
    }
    g_assets[g_asset_count].asset_id = id;
    strncpy(g_assets[g_asset_count].name, name, ASSET_NAME_LEN - 1);
    g_assets[g_asset_count].name[ASSET_NAME_LEN - 1] = '\0';
    strncpy(g_assets[g_asset_count].status, status, ASSET_STATUS_LEN - 1);
    g_assets[g_asset_count].status[ASSET_STATUS_LEN - 1] = '\0';
    ++g_asset_count;
    printf("Added Asset %d.\n", id);
}

static void asset_update(int id, const char *newName, const char *newStatus) {
    int idx = asset_search_index(id);
    if (idx < 0) {
        printf("Asset %d not found.\n", id);
        return;
    }
    if (newName && *newName) {
        strncpy(g_assets[idx].name, newName, ASSET_NAME_LEN - 1);
        g_assets[idx].name[ASSET_NAME_LEN - 1] = '\0';
    }
    if (newStatus && *newStatus) {
        strncpy(g_assets[idx].status, newStatus, ASSET_STATUS_LEN - 1);
        g_assets[idx].status[ASSET_STATUS_LEN - 1] = '\0';
    }
    printf("Updated Asset %d.\n", id);
    asset_display_one(&g_assets[idx]);
}

static void asset_count_by_status(void) {
    int active = 0, inactive = 0;
    for (int i = 0; i < g_asset_count; ++i) {
        if (strcmp(g_assets[i].status, "Active") == 0) active++;
        else inactive++;
    }
    printf("Active Assets   : %d\n", active);
    printf("Inactive Assets : %d\n", inactive);
}

void assetInventoryMenu(void) {
    int choice;
    while (1) {
        printf("\n====== Asset Inventory Management ======\n");
        printf("1. Add Asset\n");
        printf("2. Update Asset\n");
        printf("3. Search Asset\n");
        printf("4. View All Assets\n");
        printf("5. Count by Status\n");
        printf("6. Back\n");
        printf("Enter your choice: ");
        if (scanf("%d", &choice) != 1) { while (getchar()!='\n'); continue; }
        if (choice == 6) return;
        int id; char name[ASSET_NAME_LEN]; char status[ASSET_STATUS_LEN];
        switch (choice) {
            case 1:
                printf("Enter Asset ID: "); scanf("%d", &id);
                getchar();
                printf("Enter Name: "); fgets(name, sizeof(name), stdin); name[strcspn(name,"\n")] = '\0';
                printf("Enter Status (Active/Inactive): "); scanf(" %15s", status);
                asset_add(id, name, status);
                break;
            case 2:
                printf("Enter Asset ID to update: "); scanf("%d", &id); getchar();
                printf("Enter New Name (or '-' to skip): "); fgets(name, sizeof(name), stdin); name[strcspn(name,"\n")] = '\0';
                printf("Enter New Status (Active/Inactive or '-' to skip): "); scanf(" %15s", status);
                asset_update(id, (strcmp(name,"-")==0?NULL:name), (strcmp(status,"-")==0?NULL:status));
                break;
            case 3: {
                printf("Enter Asset ID to search: "); scanf("%d", &id);
                int idx = asset_search_index(id);
                if (idx >= 0) { printf("Found: "); asset_display_one(&g_assets[idx]); }
                else printf("Not found.\n");
            } break;
            case 4: asset_display_all(); break;
            case 5: asset_count_by_status(); break;
            default: printf("Invalid choice.\n");
        }
    }
}

/* =======================================================================
   BST LOAD MANAGER
   ======================================================================= */
typedef struct BSTNode {
    int id;
    int load;
    struct BSTNode *left, *right;
} BSTNode;

static BSTNode* bst_new(int id, int load) {
    BSTNode* n = (BSTNode*)malloc(sizeof(BSTNode));
    if (!n) { perror("malloc"); exit(1); }
    n->id = id; n->load = load; n->left = n->right = NULL;
    return n;
}

static BSTNode* bst_insert(BSTNode* root, int id, int load) {
    if (!root) return bst_new(id, load);
    if (id < root->id) root->left = bst_insert(root->left, id, load);
    else if (id > root->id) root->right = bst_insert(root->right, id, load);
    else root->load = load; /* update load if id exists */
    return root;
}

static BSTNode* bst_min_node(BSTNode* root) {
    while (root && root->left) root = root->left;
    return root;
}

static BSTNode* bst_delete(BSTNode* root, int id) {
    if (!root) return NULL;
    if (id < root->id) root->left = bst_delete(root->left, id);
    else if (id > root->id) root->right = bst_delete(root->right, id);
    else {
        if (!root->left) { BSTNode* r = root->right; free(root); return r; }
        if (!root->right){ BSTNode* l = root->left;  free(root); return l; }
        BSTNode* succ = bst_min_node(root->right);
        root->id = succ->id; root->load = succ->load;
        root->right = bst_delete(root->right, succ->id);
    }
    return root;
}

static BSTNode* bst_find(BSTNode* root, int id) {
    while (root) {
        if (id < root->id) root = root->left;
        else if (id > root->id) root = root->right;
        else return root;
    }
    return NULL;
}

static void bst_inorder(BSTNode* root) {
    if (!root) return;
    bst_inorder(root->left);
    printf("Server ID: %d | Load: %d\n", root->id, root->load);
    bst_inorder(root->right);
}

static BSTNode* bst_min_load(BSTNode* root) {
    if (!root) return NULL;
    BSTNode* left = bst_min_load(root->left);
    BSTNode* right= bst_min_load(root->right);
    BSTNode* best = root;
    if (left && left->load < best->load) best = left;
    if (right && right->load < best->load) best = right;
    return best;
}

static BSTNode* bst_max_load(BSTNode* root) {
    if (!root) return NULL;
    BSTNode* left = bst_max_load(root->left);
    BSTNode* right= bst_max_load(root->right);
    BSTNode* best = root;
    if (left && left->load > best->load) best = left;
    if (right && right->load > best->load) best = right;
    return best;
}

void bstLoadManagerMenu(void) {
    BSTNode* root = NULL;
    int choice;
    while (1) {
        printf("\n====== BST Load Manager ======\n");
        printf("1. Insert/Update Server (ID, Load)\n");
        printf("2. Delete Server by ID\n");
        printf("3. Display All Servers (Inorder by ID)\n");
        printf("4. Find Server by ID\n");
        printf("5. Show Min/Max Load Server\n");
        printf("6. Back\n");
        printf("Enter your choice: ");
        if (scanf("%d", &choice) != 1) { while (getchar()!='\n'); continue; }
        if (choice == 6) return;
        int id, load;
        switch (choice) {
            case 1:
                printf("Enter ID and Load: ");
                scanf("%d %d", &id, &load);
                root = bst_insert(root, id, load);
                printf("Upserted server %d.\n", id);
                break;
            case 2:
                printf("Enter ID to delete: ");
                scanf("%d", &id);
                root = bst_delete(root, id);
                printf("Deleted (if existed).\n");
                break;
            case 3:
                if (!root) printf("(empty)\n"); else bst_inorder(root);
                break;
            case 4: {
                printf("Enter ID to find: ");
                scanf("%d", &id);
                BSTNode* n = bst_find(root, id);
                if (n) printf("Found -> ID: %d | Load: %d\n", n->id, n->load);
                else printf("Not found.\n");
            } break;
            case 5: {
                BSTNode* mn = bst_min_load(root);
                BSTNode* mx = bst_max_load(root);
                if (mn) printf("Min load -> ID: %d | Load: %d\n", mn->id, mn->load);
                else printf("No servers.\n");
                if (mx) printf("Max load -> ID: %d | Load: %d\n", mx->id, mx->load);
            } break;
            default: printf("Invalid choice.\n");
        }
    }
}

/* =======================================================================
   HEAP MANAGER (Min-Heap)
   ======================================================================= */
#define HEAP_MAX 100

typedef struct {
    int task_id;
    char task_name[50];
    int priority; // Lower = More urgent
} HeapTask;

typedef struct {
    HeapTask heap[HEAP_MAX];
    int size;
} MinHeap;

static void heap_swap(HeapTask *a, HeapTask *b) {
    HeapTask tmp = *a;
    *a = *b;
    *b = tmp;
}

static void heap_init(MinHeap *h) {
    h->size = 0;
}

static void heap_insert(MinHeap *h, int id, const char *name, int priority) {
    if (h->size >= HEAP_MAX) {
        printf("Error: Scheduler is full. Cannot add more tasks.\n");
        return;
    }

    HeapTask t;
    t.task_id = id;
    strncpy(t.task_name, name, sizeof(t.task_name)-1);
    t.task_name[sizeof(t.task_name)-1] = '\0';
    t.priority = priority;

    int i = h->size++;
    h->heap[i] = t;

    /* bubble up */
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (h->heap[parent].priority <= h->heap[i].priority) break;
        heap_swap(&h->heap[i], &h->heap[parent]);
        i = parent;
    }
    printf("✅ Task '%s' added successfully with priority %d.\n", name, priority);
}

static void heapify_down(MinHeap *h, int i) {
    while (1) {
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        int smallest = i;

        if (left < h->size && h->heap[left].priority < h->heap[smallest].priority) smallest = left;
        if (right < h->size && h->heap[right].priority < h->heap[smallest].priority) smallest = right;

        if (smallest == i) break;
        heap_swap(&h->heap[i], &h->heap[smallest]);
        i = smallest;
    }
}

static HeapTask heap_extract_min(MinHeap *h) {
    if (h->size == 0) {
        HeapTask empty = {-1, "None", -1};
        printf("⚠️ No tasks available to extract.\n");
        return empty;
    }
    HeapTask root = h->heap[0];
    h->heap[0] = h->heap[h->size - 1];
    h->size--;
    heapify_down(h, 0);
    printf("🛠️ Extracted Task: %s (Priority %d)\n", root.task_name, root.priority);
    return root;
}

static void heap_display(MinHeap *h) {
    if (h->size == 0) { printf("📋 No maintenance tasks scheduled.\n"); return; }
    printf("\n=== Current Scheduled Maintenance Tasks ===\n");
    printf("ID\tTask Name\t\tPriority\n");
    printf("--------------------------------------------\n");
    for (int i = 0; i < h->size; ++i) {
        printf("%d\t%-20s\t%d\n", h->heap[i].task_id, h->heap[i].task_name, h->heap[i].priority);
    }
    printf("--------------------------------------------\n");
}

void heapSchedulerMenu(void) {
    MinHeap scheduler;
    heap_init(&scheduler);

    int choice, id, priority;
    char name[50];

    while (1) {
        printf("\n====== Heap Scheduler (Min-Heap) ======\n");
        printf("1. Add Task\n");
        printf("2. View Tasks\n");
        printf("3. Extract Most Urgent Task\n");
        printf("4. Back\n");
        printf("Enter your choice: ");
        if (scanf("%d", &choice) != 1) { while (getchar()!='\n'); continue; }
        if (choice == 4) return;
        switch (choice) {
            case 1:
                printf("Enter Task ID: "); scanf("%d", &id); getchar();
                printf("Enter Task Name: "); fgets(name, sizeof(name), stdin); name[strcspn(name,"\n")] = '\0';
                printf("Enter Priority (lower = more urgent): "); scanf("%d", &priority);
                heap_insert(&scheduler, id, name, priority);
                break;
            case 2:
                heap_display(&scheduler);
                break;
            case 3:
                heap_extract_min(&scheduler);
                break;
            default:
                printf("Invalid choice. Try again.\n");
        }
    }
}

/* =======================================================================
   SERVER HEALTH (Queue)
   ======================================================================= */
#define QMAX 32

typedef struct {
    int serverID;
    char status[20];
    float cpuUsage;
    float memoryUsage;
} ServerReport;

typedef struct {
    ServerReport reports[QMAX];
    int front, rear, count;
} Queue;

static void q_init(Queue *q) { q->front = 0; q->rear = -1; q->count = 0; }
static int q_full(Queue *q)  { return q->count == QMAX; }
static int q_empty(Queue *q) { return q->count == 0; }

static void enqueue(Queue *q, ServerReport r) {
    if (q_full(q)) { printf("Queue FULL.\n"); return; }
    q->rear = (q->rear + 1) % QMAX;
    q->reports[q->rear] = r;
    q->count++;
    printf("Report for Server %d added.\n", r.serverID);
}

static void dequeue(Queue *q) {
    if (q_empty(q)) { printf("Queue EMPTY.\n"); return; }
    ServerReport r = q->reports[q->front];
    q->front = (q->front + 1) % QMAX;
    q->count--;
    printf("\nProcessed Report:\n");
    printf("Server ID: %d\nStatus: %s\nCPU: %.2f%%\nMEM: %.2f%%\n", r.serverID, r.status, r.cpuUsage, r.memoryUsage);
}

static void q_display(Queue *q) {
    if (q_empty(q)) { printf("(no reports)\n"); return; }
    printf("\n--- Reports in Queue ---\n");
    for (int i = 0, idx = q->front; i < q->count; ++i, idx = (idx + 1) % QMAX) {
        ServerReport *r = &q->reports[idx];
        printf("[%d] ID:%d | %s | CPU:%.2f%% | MEM:%.2f%%\n", i+1, r->serverID, r->status, r->cpuUsage, r->memoryUsage);
    }
}

static void getValidStatus(char *buf) {
    while (1) {
        printf("Enter Status (Healthy/Warning/Critical): ");
        scanf(" %19s", buf);
        if (strcmp(buf, "Healthy")==0 || strcmp(buf, "Warning")==0 || strcmp(buf,"Critical")==0) return;
        printf("Invalid status. Try again.\n");
    }
}

void serverHealthMenu(void) {
    Queue q; q_init(&q);
    int choice;
    while (1) {
        printf("\n====== Server Health Queue ======\n");
        printf("1. Add Server Report\n");
        printf("2. Process Next Report\n");
        printf("3. Display All Reports\n");
        printf("4. Back\n");
        printf("Enter your choice: ");
        if (scanf("%d", &choice) != 1) { while (getchar()!='\n'); continue; }
        if (choice == 4) return;
        ServerReport r;
        switch (choice) {
            case 1:
                if (q_full(&q)) { printf("Queue FULL.\n"); break; }
                printf("Enter Server ID: "); scanf("%d", &r.serverID);
                getValidStatus(r.status);
                printf("Enter CPU Usage (%%): "); scanf("%f", &r.cpuUsage);
                printf("Enter Memory Usage (%%): "); scanf("%f", &r.memoryUsage);
                enqueue(&q, r);
                break;
            case 2:
                dequeue(&q);
                break;
            case 3:
                q_display(&q);
                break;
            default: printf("Invalid choice.\n");
        }
    }
}