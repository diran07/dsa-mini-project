#include <stdio.h>
#include <stdlib.h>

/* Type definition must match functions.c */
typedef enum {
    ROLE_NONE = 0,
    ROLE_CLIENT = 1,
    ROLE_ADMIN = 2
} UserRole;

/* Declare external variable from functions.c */
extern UserRole current_role;

/* Menus implemented in functions.c */
void assetInventoryMenu(void);
void bstLoadManagerMenu(void);
void heapSchedulerMenu(void);
void serverHealthMenu(void);

/* Simple role selection function */
UserRole selectRole(void) {
    int choice;
    
    printf("\n========== SELECT ROLE ==========\n");
    printf("1. Login as Admin\n");
    printf("2. Login as Client\n");
    printf("3. Exit\n");
    printf("=================================\n");
    printf("Enter your choice: ");
    
    if (scanf("%d", &choice) != 1) {
        while (getchar() != '\n');
        return ROLE_NONE;
    }
    
    switch (choice) {
        case 1:
            printf("\nLogged in as ADMINISTRATOR\n");
            return ROLE_ADMIN;
        case 2:
            printf("\nLogged in as CLIENT\n");
            return ROLE_CLIENT;
        case 3:
            printf("Exiting...\n");
            exit(0);
        default:
            printf("Invalid choice!\n");
            return ROLE_NONE;
    }
}

int main(void) {
    int choice;
    
    // First, select role
    while (1) {
        current_role = selectRole();
        
        if (current_role == ROLE_NONE) {
            printf("Please select a valid role.\n");
            continue;
        }
        
        // Once role is selected, show the main menu
        while (1) {
            printf("\n==============================\n");
            printf("        MAIN PROJECT MENU     \n");
            printf("==============================\n");
            printf("Logged in as: %s\n", current_role == ROLE_ADMIN ? "ADMINISTRATOR" : "CLIENT");
            printf("------------------------------\n");
            printf("1. Asset Inventory\n");
            printf("2. BST Load Manager\n");
            printf("3. Heap Manager\n");
            printf("4. Server Health\n");
            printf("5. Change Role\n");
            printf("6. Exit\n");
            printf("Enter your choice: ");
            
            if (scanf("%d", &choice) != 1) { 
                while (getchar() != '\n'); 
                continue; 
            }
            
            if (choice == 5) {
                printf("Changing role...\n");
                current_role = ROLE_NONE;
                break;  // Go back to role selection
            }
            
            switch (choice) {
                case 1: assetInventoryMenu(); break;
                case 2: bstLoadManagerMenu(); break;
                case 3: heapSchedulerMenu(); break;
                case 4: serverHealthMenu(); break;
                case 6: printf("Bye!\n"); return 0;
                default: printf("Invalid choice.\n");
            }
        }
    }
}
