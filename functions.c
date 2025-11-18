#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define ASSET_MAX 200
#define ASSET_NAME_LEN 64
#define ASSET_STATUS_LEN 16

typedef struct {
    int asset_id;
    char name[ASSET_NAME_LEN];
    char status[ASSET_STATUS_LEN]; /* "active" or "inactive" */
} Asset;

static Asset g_assets[ASSET_MAX];
static int g_asset_count = 0;

/* --- Utility: lowercase conversion --- */
static void to_lowercase(char *s) {
    for (int i = 0; s[i]; ++i)
        s[i] = (char)tolower((unsigned char)s[i]);
}

/* --- Display Functions --- */
static void asset_display_one(const Asset *a) {
    printf("Asset ID: %d | Name: %s | Status: %s\n",
           a->asset_id, a->name, a->status);
}

static void asset_display_all(void) {
    printf("\n--- Current Asset Inventory ---\n");
    if (g_asset_count == 0) {
        printf("(empty)\n");
        return;
    }

    printf("%-10s %-20s %-10s\n", "Asset ID", "Name", "Status");
    for (int i = 0; i < g_asset_count; ++i) {
        printf("%-10d %-20s %-10s\n",
               g_assets[i].asset_id,
               g_assets[i].name,
               g_assets[i].status);
    }
}

/* --- Core Functions --- */
static int asset_search_index(int id) {
    for (int i = 0; i < g_asset_count; ++i) {
        if (g_assets[i].asset_id == id)
            return i;
    }
    return -1;
}

static int validate_id(int id) {
    return (id >= 1000 && id <= 9999);
}

static int validate_status(const char *status) {
    char s[ASSET_STATUS_LEN];
    strncpy(s, status, sizeof(s) - 1);
    s[sizeof(s) - 1] = '\0';
    to_lowercase(s);
    return (strcmp(s, "active") == 0 || strcmp(s, "inactive") == 0);
}

static void asset_add(int id, const char *name, const char *status) {
    if (g_asset_count >= ASSET_MAX) {
        printf(" Inventory full! Cannot add more assets.\n");
        return;
    }
    if (asset_search_index(id) >= 0) {
        printf(" Asset with ID %d already exists. Use update instead.\n", id);
        return;
    }

    char status_lower[ASSET_STATUS_LEN];
    strncpy(status_lower, status, sizeof(status_lower) - 1);
    status_lower[sizeof(status_lower) - 1] = '\0';
    to_lowercase(status_lower);

    if (!validate_status(status_lower)) {
        printf(" Invalid status! Please enter 'Active' or 'Inactive'.\n");
        return;
    }

    g_assets[g_asset_count].asset_id = id;
    strncpy(g_assets[g_asset_count].name, name, ASSET_NAME_LEN - 1);
    g_assets[g_asset_count].name[ASSET_NAME_LEN - 1] = '\0';
    strncpy(g_assets[g_asset_count].status, status_lower, ASSET_STATUS_LEN - 1);
    g_assets[g_asset_count].status[ASSET_STATUS_LEN - 1] = '\0';

    ++g_asset_count;
    printf(" Added Asset %d successfully!\n", id);
}

static void asset_update(int id, const char *newName, const char *newStatus) {
    int idx = asset_search_index(id);
    if (idx < 0) {
        printf(" Asset %d not found.\n", id);
        return;
    }

    if (newName && *newName) {
        strncpy(g_assets[idx].name, newName, ASSET_NAME_LEN - 1);
        g_assets[idx].name[ASSET_NAME_LEN - 1] = '\0';
    }

    if (newStatus && *newStatus) {
        char lowerStatus[ASSET_STATUS_LEN];
        strncpy(lowerStatus, newStatus, sizeof(lowerStatus) - 1);
        lowerStatus[sizeof(lowerStatus) - 1] = '\0';
        to_lowercase(lowerStatus);

        if (!validate_status(lowerStatus)) {
            printf(" Invalid status! Please enter 'Active' or 'Inactive'.\n");
            return;
        }

        strncpy(g_assets[idx].status, lowerStatus, ASSET_STATUS_LEN - 1);
        g_assets[idx].status[ASSET_STATUS_LEN - 1] = '\0';
    }

    printf(" Updated Asset %d successfully!\n", id);
    asset_display_one(&g_assets[idx]);
}

static void asset_count_by_status(void) {
    int active = 0, inactive = 0;

    for (int i = 0; i < g_asset_count; ++i) {
        if (strcmp(g_assets[i].status, "active") == 0)
            active++;
        else if (strcmp(g_assets[i].status, "inactive") == 0)
            inactive++;
    }

    printf("\n Asset Summary:\n");
    printf("Active Assets   : %d\n", active);
    printf("Inactive Assets : %d\n", inactive);
}

/* --- Menu Function --- */
void assetInventoryMenu(void) {
    int choice;

    while (1) {
        printf("\n================= ASSET INVENTORY MANAGEMENT =================\n");
        printf(" Tip: Before updating or searching, use option 4 to view all assets.\n");
        printf("----------------------------------------------------------------\n");
        printf("1  Add a new Asset\n");
        printf("2  Update an existing Asset\n");
        printf("3  Search for an Asset\n");
        printf("4  View all Assets\n");
        printf("5  View Asset Count by Status\n");
        printf("6  Exit the Program\n");
        printf("================================================================\n");
        printf(" Enter your choice (1-6): ");

        if (scanf("%d", &choice) != 1) {
            printf(" Invalid input! Please enter a number between 1 and 6.\n");
            while (getchar() != '\n');
            continue;
        }

        if (choice == 6) {
            printf("\n Exiting Asset Management System...\n");
            return;
        }

        int id;
        char name[ASSET_NAME_LEN];
        char status[ASSET_STATUS_LEN];

        switch (choice) {
        case 1:
            printf("\n➡ Adding a new asset...\n");
            printf("Enter a 4-digit Asset ID (1000–9999): ");
            if (scanf("%d", &id) != 1 || !validate_id(id)) {
                printf(" Invalid ID! Please enter a 4-digit number between 1000 and 9999.\n");
                while (getchar() != '\n');
                break;
            }
            getchar();

            printf("Enter Asset Name: ");
            fgets(name, sizeof(name), stdin);
            name[strcspn(name, "\n")] = '\0';

            printf("Enter Status (Active/Inactive): ");
            scanf(" %15s", status);
            to_lowercase(status);

            asset_add(id, name, status);
            break;

        case 2:
            printf("\n➡ Updating an existing asset...\n");
            printf("Enter existing 4-digit Asset ID: ");
            if (scanf("%d", &id) != 1 || !validate_id(id)) {
                printf(" Invalid ID! Please enter a 4-digit number between 1000 and 9999.\n");
                while (getchar() != '\n');
                break;
            }
            getchar();

            printf("Enter New Name (or '-' to skip): ");
            fgets(name, sizeof(name), stdin);
            name[strcspn(name, "\n")] = '\0';

            printf("Enter New Status (Active/Inactive or '-' to skip): ");
            scanf(" %15s", status);

            if (strcmp(status, "-") != 0)
                to_lowercase(status);

            asset_update(id, (strcmp(name, "-") == 0 ? NULL : name),
                              (strcmp(status, "-") == 0 ? NULL : status));
            break;

        case 3: {
            printf("\n➡ Searching for an asset...\n");
            printf("Enter 4-digit Asset ID to search: ");
            if (scanf("%d", &id) != 1 || !validate_id(id)) {
                printf(" Invalid ID! Please enter a 4-digit number between 1000 and 9999.\n");
                while (getchar() != '\n');
                break;
            }

            int idx = asset_search_index(id);
            if (idx >= 0) {
                printf("\n Asset found:\n");
                asset_display_one(&g_assets[idx]);
            } else
                printf(" Asset not found.\n");
        } break;

        case 4:
            printf("\n Displaying all assets...\n");
            asset_display_all();
            break;

        case 5:
            asset_count_by_status();
            break;

        default:
            printf(" Invalid choice! Please select a number between 1 and 6.\n");
        }
    }
}

/* =======================================================================
   BST LOAD MANAGER (Clean & User-Friendly Version)
   ======================================================================= */
#include <stdio.h>
#include <stdlib.h>

typedef struct BSTNode
{
    int id;
    int load;
    struct BSTNode *left, *right;
} BSTNode;

static BSTNode *bst_new(int id, int load)
{
    BSTNode *n = (BSTNode *)malloc(sizeof(BSTNode));
    if (!n)
    {
        perror("malloc");
        exit(1);
    }
    n->id = id;
    n->load = load;
    n->left = n->right = NULL;
    return n;
}

static BSTNode *bst_insert(BSTNode *root, int id, int load)
{
    if (!root)
        return bst_new(id, load);
    if (id < root->id)
        root->left = bst_insert(root->left, id, load);
    else if (id > root->id)
        root->right = bst_insert(root->right, id, load);
    else
        root->load = load; /* update load if id exists */
    return root;
}

static BSTNode *bst_min_node(BSTNode *root)
{
    while (root && root->left)
        root = root->left;
    return root;
}

static BSTNode *bst_delete(BSTNode *root, int id)
{
    if (!root)
        return NULL;
    if (id < root->id)
        root->left = bst_delete(root->left, id);
    else if (id > root->id)
        root->right = bst_delete(root->right, id);
    else
    {
        if (!root->left)
        {
            BSTNode *r = root->right;
            free(root);
            return r;
        }
        if (!root->right)
        {
            BSTNode *l = root->left;
            free(root);
            return l;
        }
        BSTNode *succ = bst_min_node(root->right);
        root->id = succ->id;
        root->load = succ->load;
        root->right = bst_delete(root->right, succ->id);
    }
    return root;
}

static BSTNode *bst_find(BSTNode *root, int id)
{
    while (root)
    {
        if (id < root->id)
            root = root->left;
        else if (id > root->id)
            root = root->right;
        else
            return root;
    }
    return NULL;
}

static void bst_inorder(BSTNode *root)
{
    if (!root)
        return;
    bst_inorder(root->left);
    printf("Server ID: %d | Load: %d%%\n", root->id, root->load);
    bst_inorder(root->right);
}

static BSTNode *bst_min_load(BSTNode *root)
{
    if (!root)
        return NULL;
    BSTNode *left = bst_min_load(root->left);
    BSTNode *right = bst_min_load(root->right);
    BSTNode *best = root;
    if (left && left->load < best->load)
        best = left;
    if (right && right->load < best->load)
        best = right;
    return best;
}

static BSTNode *bst_max_load(BSTNode *root)
{
    if (!root)
        return NULL;
    BSTNode *left = bst_max_load(root->left);
    BSTNode *right = bst_max_load(root->right);
    BSTNode *best = root;
    if (left && left->load > best->load)
        best = left;
    if (right && right->load > best->load)
        best = right;
    return best;
}

void bstLoadManagerMenu(void)
{
    BSTNode *root = NULL;
    int choice;
    while (1)
    {
        printf("\n====== BST LOAD MANAGER ======\n");
        printf("Note:\n");
        printf("  - Server ID = Unique integer (e.g. 101)\n");
        printf("  - Load = Percentage value (0-100)\n");
        printf("=================================\n");
        printf("1. Insert or Update Server\n");
        printf("2. Delete Server by ID\n");
        printf("3. Display All Servers (Inorder by ID)\n");
        printf("4. Find Server by ID\n");
        printf("5. Show Min/Max Load Server\n");
        printf("6. Back\n");
        printf("---------------------------------\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1)
        {
            printf("Invalid input. Please enter a number.\n");
            while (getchar() != '\n')
                ;
            continue;
        }

        if (choice == 6)
        {
            printf("Exiting BST Load Manager...\n");
            return;
        }

        int id, load;
        switch (choice)
        {
        case 1:
            printf("\nEnter Server ID (integer): ");
            if (scanf("%d", &id) != 1)
            {
                printf("Invalid ID.\n");
                while (getchar() != '\n')
                    ;
                break;
            }
            printf("Enter Server Load (0-100): ");
            if (scanf("%d", &load) != 1 || load < 0 || load > 100)
            {
                printf("Invalid load value. Must be between 0 and 100.\n");
                while (getchar() != '\n')
                    ;
                break;
            }
            root = bst_insert(root, id, load);
            printf("Server %d added/updated successfully with %d%% load.\n", id, load);
            break;

        case 2:
            printf("Enter Server ID to delete: ");
            if (scanf("%d", &id) != 1)
            {
                printf("Invalid input.\n");
                while (getchar() != '\n')
                    ;
                break;
            }
            root = bst_delete(root, id);
            printf("Deleted server %d (if it existed).\n", id);
            break;

        case 3:
            if (!root)
                printf("No servers available.\n");
            else
            {
                printf("\nCurrent Servers (sorted by ID):\n");
                bst_inorder(root);
            }
            break;

        case 4:
            printf("Enter Server ID to search: ");
            if (scanf("%d", &id) != 1)
            {
                printf("Invalid input.\n");
                while (getchar() != '\n')
                    ;
                break;
            }
            {
                BSTNode *n = bst_find(root, id);
                if (n)
                    printf("Found -> ID: %d | Load: %d%%\n", n->id, n->load);
                else
                    printf("Server with ID %d not found.\n", id);
            }
            break;

        case 5:
        {
            BSTNode *mn = bst_min_load(root);
            BSTNode *mx = bst_max_load(root);
            if (!mn)
                printf("No servers to analyze.\n");
            else
            {
                printf("Minimum Load -> ID: %d | Load: %d%%\n", mn->id, mn->load);
                printf("Maximum Load -> ID: %d | Load: %d%%\n", mx->id, mx->load);
            }
        }
        break;

        default:
            printf("Invalid choice. Please select a number between 1 and 6.\n");
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

/* =======================================================================
   UTILITY FUNCTIONS
   ======================================================================= */
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
        printf("  Error: Scheduler is full. Cannot add more tasks.\n");
        return;
    }

    HeapTask t;
    t.task_id = id;
    strncpy(t.task_name, name, sizeof(t.task_name) - 1);
    t.task_name[sizeof(t.task_name) - 1] = '\0';
    t.priority = priority;

    int i = h->size++;
    h->heap[i] = t;

    /* bubble up */
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (h->heap[parent].priority <= h->heap[i].priority)
            break;
        heap_swap(&h->heap[i], &h->heap[parent]);
        i = parent;
    }

    printf(" Task '%s' added successfully with priority %d.\n", name, priority);
}

static void heapify_down(MinHeap *h, int i) {
    while (1) {
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        int smallest = i;

        if (left < h->size && h->heap[left].priority < h->heap[smallest].priority)
            smallest = left;
        if (right < h->size && h->heap[right].priority < h->heap[smallest].priority)
            smallest = right;

        if (smallest == i)
            break;
        heap_swap(&h->heap[i], &h->heap[smallest]);
        i = smallest;
    }
}

static HeapTask heap_extract_min(MinHeap *h) {
    if (h->size == 0) {
        HeapTask empty = {-1, "None", -1};
        printf("  No tasks available to extract.\n");
        return empty;
    }
    HeapTask root = h->heap[0];
    h->heap[0] = h->heap[h->size - 1];
    h->size--;
    heapify_down(h, 0);
    printf("\n  Extracted Most Urgent Task:\n");
    printf("    Task ID: %d\n", root.task_id);
    printf("    Task Name: %s\n", root.task_name);
    printf("    Priority: %d\n", root.priority);
    return root;
}

static void heap_display(MinHeap *h) {
    if (h->size == 0) {
        printf("\n No maintenance tasks scheduled.\n");
        return;
    }

    printf("\n=== Current Scheduled Maintenance Tasks ===\n");
    printf("%-5s %-25s %-10s\n", "ID", "Task Name", "Priority");
    printf("--------------------------------------------\n");
    for (int i = 0; i < h->size; ++i) {
        printf("%-5d %-25s %-10d\n", h->heap[i].task_id, h->heap[i].task_name, h->heap[i].priority);
    }
    printf("--------------------------------------------\n");
}

/* =======================================================================
   USER-FRIENDLY MENU
   ======================================================================= */
void heapSchedulerMenu(void) {
    MinHeap scheduler;
    heap_init(&scheduler);

    int choice, id, priority;
    char name[50];

    printf("\n=============================================\n");
    printf(" Welcome to the Maintenance Task Scheduler \n");
    printf("=============================================\n");
    printf("This system helps you manage maintenance tasks.\n");
    printf("Lower priority number = More urgent task.\n");

    while (1) {
        printf("\n====== Heap Scheduler Menu ======\n");
        printf("1  Add a New Task\n");
        printf("2 View All Scheduled Tasks\n");
        printf("3 Extract the Most Urgent Task\n");
        printf("4 Exit Scheduler\n");
        printf("=================================\n");
        printf("Enter your choice (1-4): ");

        if (scanf("%d", &choice) != 1) {
            printf(" Invalid input! Please enter a number (1-4).\n");
            while (getchar() != '\n');
            continue;
        }

        if (choice == 4) {
            printf(" Exiting Heap Scheduler. Goodbye!\n");
            return;
        }

        switch (choice) {
            case 1:
                printf("\n Adding a New Task:\n");
                printf(" Enter a numeric Task ID (e.g., 101): ");
                if (scanf("%d", &id) != 1) {
                    printf("Invalid input! Task ID must be a number.\n");
                    while (getchar() != '\n');
                    break;
                }
                getchar(); // clear newline
                printf(" Enter a short Task Name (e.g., 'Oil Check'): ");
                fgets(name, sizeof(name), stdin);
                name[strcspn(name, "\n")] = '\0';
                printf("  Enter Priority (1 = highest urgency, higher = less urgent): ");
                if (scanf("%d", &priority) != 1) {
                    printf("Invalid input! Priority must be a number.\n");
                    while (getchar() != '\n');
                    break;
                }
                heap_insert(&scheduler, id, name, priority);
                break;

            case 2:
                heap_display(&scheduler);
                break;

            case 3:
                heap_extract_min(&scheduler);
                printf("\n");
                printf("current task available now :\n");
                heap_display(&scheduler);
                break;

            default:
                printf("Invalid choice! Please enter 1-4.\n");
        }
    }
}


/* =======================================================================
   SERVER HEALTH (Queue)  -- CLEANED AND DEDUPLICATED
   ======================================================================= */

#define QMAX 32

struct ServerReport {
    int serverID;
    char status[20];
    float cpuUsage;
    float memoryUsage;
};

typedef struct {
    struct ServerReport reports[QMAX];
    int front, rear, count;
} Queue;

/* Queue Helper Functions */
static void q_init(Queue *q) { q->front = 0; q->rear = -1; q->count = 0; }
static int q_full(Queue *q)  { return q->count == QMAX; }
static int q_empty(Queue *q) { return q->count == 0; }

static void enqueue(Queue *q, struct ServerReport r) {
    if (q_full(q)) {
        printf("Queue is FULL! Cannot add more reports.\n");
        return;
    }
    q->rear = (q->rear + 1) % QMAX;
    q->reports[q->rear] = r;
    q->count++;
    printf("Report for Server %d added successfully.\n", r.serverID);
}

static void dequeue(Queue *q) {
    if (q_empty(q)) {
        printf("Queue is EMPTY! No reports to process.\n");
        return;
    }

    struct ServerReport r = q->reports[q->front];
    q->front = (q->front + 1) % QMAX;
    q->count--;

    printf("\nProcessed Report:\n");
    printf("------------------------------\n");
    printf("Server ID : %d\n", r.serverID);
    printf("Status    : %s\n", r.status);
    printf("CPU Usage : %.2f%%\n", r.cpuUsage);
    printf("Memory    : %.2f%%\n", r.memoryUsage);
    printf("------------------------------\n");
}

static void q_display(Queue *q) {
    if (q_empty(q)) {
        printf("No reports in queue.\n");
        return;
    }

    printf("\nReports in Queue (%d total):\n", q->count);
    printf("---------------------------------------------------------------\n");
    printf("| No | Server ID |   Status    | CPU(%%) | MEM(%%) |\n");
    printf("---------------------------------------------------------------\n");

    for (int i = 0, idx = q->front; i < q->count; i++, idx = (idx + 1) % QMAX) {
        struct ServerReport *r = &q->reports[idx];
        printf("| %2d | %9d | %-11s | %6.2f | %6.2f |\n",
               i + 1, r->serverID, r->status, r->cpuUsage, r->memoryUsage);
    }
    printf("---------------------------------------------------------------\n");
}

/* Convert status to Title Case (healthy -> Healthy) */
void toTitleCase(char *s) {
    s[0] = toupper(s[0]);
    for (int i = 1; s[i]; i++)
        s[i] = tolower(s[i]);
}

/* Status Input Validation */
static void getValidStatus(char *buf) {
    while (1) {
        printf("Enter Status (Healthy / Warning / Critical): ");
        scanf(" %19s", buf);

        toTitleCase(buf);

        if (strcmp(buf, "Healthy") == 0 ||
            strcmp(buf, "Warning") == 0 ||
            strcmp(buf, "Critical") == 0)
            return;

        printf("Invalid status. Try again.\n");
    }
}

/* Percentage Validation */
static float getValidPercentage(const char *msg) {
    float value;

    while (1) {
        printf("%s (0-100): ", msg);
        if (scanf("%f", &value) == 1 && value >= 0 && value <= 100)
            return value;

        printf("Invalid value. Please enter a number between 0 and 100.\n");
        while (getchar() != '\n'); 
    }
}

void serverHealthMenu(void) {
    Queue q;
    q_init(&q);
    int choice;

    while (1) {
        printf("\n=========================================\n");
        printf("         SERVER HEALTH MANAGEMENT\n");
        printf("=========================================\n");
        printf("1. Add Server Report\n");
        printf("2. Process Next Report\n");
        printf("3. Display All Reports\n");
        printf("4. Exit to Main Menu\n");
        printf("-----------------------------------------\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            printf("Invalid input. Please enter a number.\n");
            continue;
        }

        if (choice == 4) {
            printf("Exiting Server Health Menu.\n");
            break;
        }

        struct ServerReport r;

        switch (choice) {
            case 1:
                if (q_full(&q)) {
                    printf("Queue is FULL. Cannot add more reports.\n");
                    break;
                }

                printf("Enter Server ID (e.g., 101): ");
                if (scanf("%d", &r.serverID) != 1) {
                    while (getchar() != '\n');
                    printf("Invalid Server ID.\n");
                    break;
                }

                getValidStatus(r.status);
                r.cpuUsage = getValidPercentage("Enter CPU Usage");
                r.memoryUsage = getValidPercentage("Enter Memory Usage");

                enqueue(&q, r);
                break;

            case 2:
                dequeue(&q);
                break;

            case 3:
                q_display(&q);
                break;

            default:
                printf("Invalid choice. Please try again.\n");
        }

        while (getchar() != '\n');  
    }
}
