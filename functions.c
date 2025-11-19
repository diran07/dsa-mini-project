#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define ASSET_MAX 200
#define ASSET_NAME_LEN 64
#define USERNAME_LEN 32
#define PASSWORD_LEN 32
#define REQUEST_MAX 100

typedef struct {
    int asset_id;
    char name[ASSET_NAME_LEN];
    int count; /* quantity available */
} Asset;

typedef struct {
    int request_id;
    int asset_id;
    char asset_name[ASSET_NAME_LEN];
    int quantity;
    int approved; /* 0 = pending, 1 = approved, -1 = rejected */
} AssetRequest;

typedef enum {
    ROLE_NONE = 0,
    ROLE_CLIENT = 1,
    ROLE_ADMIN = 2
} UserRole;

static Asset g_assets[ASSET_MAX];
static int g_asset_count = 0;
static AssetRequest g_requests[REQUEST_MAX];
static int g_request_count = 0;
static int g_next_request_id = 1;
UserRole current_role = ROLE_NONE;

/* --- Utility: lowercase conversion --- */
static void to_lowercase_inplace(char *s) {
    for (int i = 0; s[i]; ++i)
        s[i] = (char)tolower((unsigned char)s[i]);
}

/* Return 1 if equal case-insensitive, 0 otherwise */
static int str_iequals(const char *a, const char *b) {
    char ta[ASSET_NAME_LEN];
    char tb[ASSET_NAME_LEN];

    strncpy(ta, a, sizeof(ta) - 1);
    ta[sizeof(ta) - 1] = '\0';
    strncpy(tb, b, sizeof(tb) - 1);
    tb[sizeof(tb) - 1] = '\0';

    to_lowercase_inplace(ta);
    to_lowercase_inplace(tb);

    return strcmp(ta, tb) == 0;
}

/* --- Display one Asset --- */
static void asset_display_one(const Asset *a) {
    if (a->count == 0) {
        printf("Asset ID: %d | Name: %s | Count: %d (BUYING NEW ASSETS)\n",
               a->asset_id, a->name, a->count);
    } else {
        printf("Asset ID: %d | Name: %s | Count: %d\n",
               a->asset_id, a->name, a->count);
    }
}

/* --- View All Assets --- */
static void asset_display_all(void) {
    printf("\n--- Current Asset Inventory ---\n");
    if (g_asset_count == 0) {
        printf("(empty)\n");
        return;
    }

    printf("%-10s %-30s %-10s %-20s\n", "Asset ID", "Name", "Count", "Status");
    printf("--------------------------------------------------------------------------------\n");

    for (int i = 0; i < g_asset_count; ++i) {
        printf("%-10d %-30s %-10d %-20s\n",
               g_assets[i].asset_id,
               g_assets[i].name,
               g_assets[i].count,
               g_assets[i].count == 0 ? "BUYING NEW ASSETS" : "Available");
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

static void asset_add(int id, const char *name, int count) {
    if (current_role != ROLE_ADMIN) {
        printf(" ACCESS DENIED! Only administrators can add assets.\n");
        return;
    }
    
    if (g_asset_count >= ASSET_MAX) {
        printf(" Inventory full! Cannot add more assets.\n");
        return;
    }
    if (asset_search_index(id) >= 0) {
        printf(" Asset with ID %d already exists. Use update instead.\n", id);
        return;
    }

    if (count < 0) {
        printf(" Invalid count! Count must be 0 or greater.\n");
        return;
    }

    g_assets[g_asset_count].asset_id = id;
    strncpy(g_assets[g_asset_count].name, name, ASSET_NAME_LEN - 1);
    g_assets[g_asset_count].name[ASSET_NAME_LEN - 1] = '\0';
    g_assets[g_asset_count].count = count;

    ++g_asset_count;
    printf(" Added Asset %d successfully!\n", id);
}

static void asset_update(int id, const char *newName, int newCount) {
    if (current_role != ROLE_ADMIN) {
        printf(" ACCESS DENIED! Only administrators can update assets.\n");
        return;
    }
    
    int idx = asset_search_index(id);
    if (idx < 0) {
        printf(" Asset %d not found.\n", id);
        return;
    }

    if (newName && *newName) {
        strncpy(g_assets[idx].name, newName, ASSET_NAME_LEN - 1);
        g_assets[idx].name[ASSET_NAME_LEN - 1] = '\0';
    }

    if (newCount >= 0) {
        g_assets[idx].count = newCount;
    }

    printf(" Updated Asset %d successfully!\n", id);
    asset_display_one(&g_assets[idx]);
}

/* --- Request Asset (Client) --- */
static void asset_request(void) {
    if (current_role != ROLE_CLIENT) {
        printf(" This function is for clients only.\n");
        return;
    }

    if (g_request_count >= REQUEST_MAX) {
        printf(" Request queue is full!\n");
        return;
    }

    int id, quantity;
    printf("Enter Asset ID to request: ");
    if (scanf("%d", &id) != 1 || !validate_id(id)) {
        printf("Invalid ID.\n");
        while (getchar() != '\n');
        return;
    }

    int idx = asset_search_index(id);
    if (idx < 0) {
        printf("Asset not found.\n");
        return;
    }

    printf("Enter quantity needed: ");
    if (scanf("%d", &quantity) != 1 || quantity <= 0) {
        printf("Invalid quantity.\n");
        while (getchar() != '\n');
        return;
    }

    // Create request
    g_requests[g_request_count].request_id = g_next_request_id++;
    g_requests[g_request_count].asset_id = id;
    strncpy(g_requests[g_request_count].asset_name, g_assets[idx].name, ASSET_NAME_LEN - 1);
    g_requests[g_request_count].asset_name[ASSET_NAME_LEN - 1] = '\0';
    g_requests[g_request_count].quantity = quantity;
    g_requests[g_request_count].approved = 0; // pending

    printf("\n Request submitted successfully!\n");
    printf(" Request ID: %d\n", g_requests[g_request_count].request_id);
    printf(" Asset: %s (ID: %d)\n", g_requests[g_request_count].asset_name, id);
    printf(" Quantity: %d\n", quantity);
    printf(" Status: Pending Admin Approval\n");

    ++g_request_count;
}

/* --- View Pending Requests (Admin) --- */
static void view_pending_requests(void) {
    if (current_role != ROLE_ADMIN) {
        printf(" ACCESS DENIED! Only administrators can view requests.\n");
        return;
    }

    printf("\n--- Pending Asset Requests ---\n");
    int found = 0;
    for (int i = 0; i < g_request_count; ++i) {
        if (g_requests[i].approved == 0) {
            printf("Request ID: %d | Asset: %s (ID: %d) | Quantity: %d | Status: PENDING\n",
                   g_requests[i].request_id,
                   g_requests[i].asset_name,
                   g_requests[i].asset_id,
                   g_requests[i].quantity);
            found = 1;
        }
    }
    if (!found) {
        printf("No pending requests.\n");
    }
}

/* --- Approve/Reject Request (Admin) --- */
static void process_request(void) {
    if (current_role != ROLE_ADMIN) {
        printf(" ACCESS DENIED! Only administrators can process requests.\n");
        return;
    }

    int req_id;
    printf("Enter Request ID to process: ");
    if (scanf("%d", &req_id) != 1) {
        printf("Invalid ID.\n");
        while (getchar() != '\n');
        return;
    }

    // Find request
    int req_idx = -1;
    for (int i = 0; i < g_request_count; ++i) {
        if (g_requests[i].request_id == req_id && g_requests[i].approved == 0) {
            req_idx = i;
            break;
        }
    }

    if (req_idx < 0) {
        printf("Request not found or already processed.\n");
        return;
    }

    // Find asset
    int asset_idx = asset_search_index(g_requests[req_idx].asset_id);
    if (asset_idx < 0) {
        printf("Asset no longer exists.\n");
        return;
    }

    printf("\nRequest Details:\n");
    printf("Asset: %s (ID: %d)\n", g_requests[req_idx].asset_name, g_requests[req_idx].asset_id);
    printf("Quantity Requested: %d\n", g_requests[req_idx].quantity);
    printf("Current Stock: %d\n", g_assets[asset_idx].count);

    int choice;
    printf("\n1. Approve\n2. Reject\nEnter choice: ");
    if (scanf("%d", &choice) != 1) {
        printf("Invalid choice.\n");
        while (getchar() != '\n');
        return;
    }

    if (choice == 1) {
        // Approve
        if (g_assets[asset_idx].count < g_requests[req_idx].quantity) {
            printf("\n WARNING: Insufficient stock! Current: %d, Requested: %d\n",
                   g_assets[asset_idx].count, g_requests[req_idx].quantity);
            printf("Approve anyway? (1=Yes, 0=No): ");
            int confirm;
            if (scanf("%d", &confirm) != 1 || confirm != 1) {
                printf("Request not approved.\n");
                return;
            }
        }

        g_assets[asset_idx].count -= g_requests[req_idx].quantity;
        if (g_assets[asset_idx].count < 0) {
            g_assets[asset_idx].count = 0;
        }

        g_requests[req_idx].approved = 1;
        printf("\n Request APPROVED!\n");
        printf(" Asset '%s' count reduced from %d to %d\n",
               g_assets[asset_idx].name,
               g_assets[asset_idx].count + g_requests[req_idx].quantity,
               g_assets[asset_idx].count);

        if (g_assets[asset_idx].count == 0) {
            printf(" ** ALERT: Asset count is now 0. BUYING NEW ASSETS! **\n");
        }
    } else if (choice == 2) {
        // Reject
        g_requests[req_idx].approved = -1;
        printf("\n Request REJECTED!\n");
    } else {
        printf("Invalid choice.\n");
    }
}

/* --- Summary Page: Total Count and Assets --- */
static void asset_summary(void) {
    int total_items = 0;
    int out_of_stock = 0;

    for (int i = 0; i < g_asset_count; ++i) {
        total_items += g_assets[i].count;
        if (g_assets[i].count == 0) {
            out_of_stock++;
        }
    }

    printf("\n Asset Summary:\n");
    printf("Total unique assets: %d\n", g_asset_count);
    printf("Total item count   : %d\n", total_items);
    printf("Out of stock       : %d\n", out_of_stock);
    if (out_of_stock > 0) {
        printf("\n** %d asset(s) need restocking - BUYING NEW ASSETS **\n", out_of_stock);
    }
}

/* --- Menu Function --- */
void assetInventoryMenu(void) {
    int choice;

    while (1) {
        printf("\n================= ASSET INVENTORY MANAGEMENT =================\n");
        printf("Logged in as: %s\n", current_role == ROLE_ADMIN ? "ADMINISTRATOR" : "CLIENT");
        printf("----------------------------------------------------------------\n");
        
        if (current_role == ROLE_ADMIN) {
            printf("1  Add a new Asset\n");
            printf("2  Update an existing Asset\n");
            printf("3  Search for an Asset\n");
            printf("4  View all Assets\n");
            printf("5  View Asset Summary\n");
            printf("6  View Pending Requests\n");
            printf("7  Approve/Reject Request\n");
            printf("8  Back\n");
        } else {
            printf("1  Search for an Asset\n");
            printf("2  View all Assets\n");
            printf("3  View Asset Summary\n");
            printf("4  Request an Asset\n");
            printf("5  Back\n");
        }
        
        printf("================================================================\n");
        printf(" Enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            printf(" Invalid input! Please enter a valid number.\n");
            while (getchar() != '\n');
            continue;
        }

        int id;
        char name[ASSET_NAME_LEN];
        int count;

        if (current_role == ROLE_ADMIN) {
            if (choice == 8) return;
            
            switch (choice) {
            case 1:
                printf("Enter Asset ID (1000-9999): ");
                if (scanf("%d", &id) != 1 || !validate_id(id)) {
                    printf("Invalid ID.\n");
                    while (getchar() != '\n');
                    break;
                }
                getchar();

                printf("Enter Name: ");
                fgets(name, sizeof(name), stdin);
                name[strcspn(name, "\n")] = '\0';

                printf("Enter Count: ");
                if (scanf("%d", &count) != 1) {
                    printf("Invalid count.\n");
                    while (getchar() != '\n');
                    break;
                }

                asset_add(id, name, count);
                break;

            case 2:
                printf("Enter Asset ID to update: ");
                if (scanf("%d", &id) != 1 || !validate_id(id)) {
                    printf("Invalid ID.\n");
                    while (getchar() != '\n');
                    break;
                }
                getchar();

                printf("Enter New Name (or '-' to skip): ");
                fgets(name, sizeof(name), stdin);
                name[strcspn(name, "\n")] = '\0';

                printf("Enter New Count (or -1 to skip): ");
                if (scanf("%d", &count) != 1) {
                    printf("Invalid count.\n");
                    break;
                }

                asset_update(id, (strcmp(name, "-") == 0 ? NULL : name), count);
                break;

            case 3:
                printf("Enter Asset ID to search: ");
                if (scanf("%d", &id) != 1 || !validate_id(id)) {
                    printf("Invalid ID.\n");
                    while (getchar() != '\n');
                    break;
                }

                {
                    int idx = asset_search_index(id);
                    if (idx >= 0)
                        asset_display_one(&g_assets[idx]);
                    else
                        printf("Asset not found.\n");
                }
                break;

            case 4:
                asset_display_all();
                break;

            case 5:
                asset_summary();
                break;

            case 6:
                view_pending_requests();
                break;

            case 7:
                process_request();
                break;

            default:
                printf("Invalid choice.\n");
            }
        } else { // CLIENT
            if (choice == 5) return;
            
            switch (choice) {
            case 1:
                printf("Enter Asset ID to search: ");
                if (scanf("%d", &id) != 1 || !validate_id(id)) {
                    printf("Invalid ID.\n");
                    while (getchar() != '\n');
                    break;
                }

                {
                    int idx = asset_search_index(id);
                    if (idx >= 0)
                        asset_display_one(&g_assets[idx]);
                    else
                        printf("Asset not found.\n");
                }
                break;

            case 2:
                asset_display_all();
                break;

            case 3:
                asset_summary();
                break;

            case 4:
                asset_request();
                break;

            default:
                printf("Invalid choice.\n");
            }
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
