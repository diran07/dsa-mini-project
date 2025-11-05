
#include <stdio.h>
#include <stdlib.h>

/* Menus implemented in functions.c */
void assetInventoryMenu(void);
void bstLoadManagerMenu(void);
void heapSchedulerMenu(void);
void serverHealthMenu(void);

int main(void) {
    int choice;
    while (1) {
        printf("\n==============================\n");
        printf("        MAIN PROJECT MENU     \n");
        printf("==============================\n");
        printf("1. Asset Inventory\n");
        printf("2. BST Load Manager\n");
        printf("3. Heap Manager\n");
        printf("4. Server Health\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        if (scanf("%d", &choice) != 1) { while (getchar()!='\n'); continue; }
        switch (choice) {
            case 1: assetInventoryMenu(); break;
            case 2: bstLoadManagerMenu(); break;
            case 3: heapSchedulerMenu(); break;
            case 4: serverHealthMenu(); break;
            case 5: printf("Bye!\n"); return 0;
            default: printf("Invalid choice.\n");
        }
    }
}
