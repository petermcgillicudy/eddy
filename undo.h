#include <memory>

enum Action {aInsertRow, aDelRow, aRowInsertChar, aRowAppendString, aRowDelChar};

struct UndoList
{
    void *list;
    int count;
    int size;
};


void initUndoList(UndoList *undoList)
{
    undoList->list = malloc(10);
    undoList->count = 0;
    undoList->size = 10;
}

void resetUndoList(UndoList *undoList)
{
    // Frees all actions.
    for (int i=0; i<undoList->size; i++) free(undoList[i]);

    // Reinitializes the list.
    undoList->list = realloc(undoList->list, 10);
    undoList->count = 0;
    undoList->size = 10;
}

void addUndoAction(Action action, UndoList *undoList)
{


}