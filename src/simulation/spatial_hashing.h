#ifndef _SPATIAL_HASHING_H__
#define _SPATIAL_HASHING_H__

typedef struct spacehash_grid sp_grid;

/// @brief Initialize and allocates the spatial hashing structure. Will need to be free'd with spacehash_free.
/// @param width The width of the space
/// @param height The height of the space
/// @param cell_width The width of the spatial hashing grid cell
/// @param get_element_x Function pointer that returns the x-coordinate of the element
/// @param get_element_y Function pointer that returns the y-coordinate of the element
/// @return The spatial hashing structure
sp_grid* spacehash_init(float width, float height, float cell_width, float (*get_element_x)(void *), float (*get_element_y)(void *));

/// @brief Updates the granularity of the spatial hashing grid
/// @param g Pointer on the spatial hashing structure
/// @param new_cell_width The new width
void spacehash_update_cell_width(sp_grid** g, float new_cell_width);

/// @brief Resets the spatial grid to a clean state.
/// @param g Pointer on the spatial hashing structure
void spacehash_reset(sp_grid* g);

/// @brief Adds the element to the spatial hashing grid
/// @param g Pointer on the spatial hashing structure
/// @param element Pointer on the element to add
void spacehash_add(sp_grid* g, void *element);

/// @brief Fills the "neighbors" array with pointers to the elements that are in the same cell as the given "element". NULL is used as a end delimiter, if the array gets completely filled there will be no end delimiter. This array must be of size max_neighbors.
/// @param g Pointer on the spatial hashing structure
/// @param element Pointer on the element to query around
/// @param neighbors Array that will be filled with the pointers.
/// @param max_neighbors Maximum size of the given array
void spacehash_query(sp_grid* g, void *element, void **neighbors, size_t max_neighbors);

/// @brief Frees the allocated memory for the spatial hash grid
/// @param g Pointer on the spatial hashing structure to free
void spacehash_free(sp_grid* g);

#endif // _SPATIAL_HASHING_H__
