#include <linux/gfp.h>
#include <linux/slab.h>

#include "assert.h"
#include "stack.h"

stack_entry_t* create_stack_entry(void *data)
{
    stack_entry_t* new_entry = (stack_entry_t*) kmalloc(sizeof(*new_entry), GFP_KERNEL);

    if(new_entry) {
        INIT_LIST_HEAD(&new_entry->lh);
        STACK_ENTRY_DATA_RESET(new_entry, data);
    }

    return new_entry;
}

void delete_stack_entry(stack_entry_t *entry)
{
    assert(entry);
    kfree(entry);
}

void stack_push(struct list_head *stack, stack_entry_t *entry)
{
    assert(stack);
    assert(entry);
    list_add(&entry->lh, stack);
}

stack_entry_t* stack_pop(struct list_head *stack)
{
    assert(stack);

    stack_entry_t* entry = (stack_entry_t*)list_first_entry(stack, stack_entry_t, lh);
    list_del(&entry->lh);

    return entry;
}
