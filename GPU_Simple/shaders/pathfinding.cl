typedef struct {
    float x;
    float y;
} Vector2f;

// Entity struct with position, velocity, and state
typedef struct
{
    Vector2f position;        // Current position
} Entity;

__kernel void enemyai(__global Entity* entities,
                      __global const uchar* grid,
                      int width,
                      int height,
                      float dt,
                      float2 goal_position) 
{
    int id = get_global_id(0);

    // Retrieve the entity
    Entity* entity = &entities[id];

    // Get the enemy's current position
    float enemy_x = entity->position.x;
    float enemy_y = entity->position.y;

    // Get the player's current position
    float player_x = goal_position.x;
    float player_y = goal_position.y;

    // Direction vectors to move towards the player
    int dx = 0;
    int dy = 0;

    // Calculate direction to move based on the player's position
    if (player_x > enemy_x) 
    {
        dx = 1;
    }
    else if (player_x < enemy_x) 
    {
        dx = -1;
    }

    if (player_y > enemy_y) 
    {
        dy = 1;
    }
    else if (player_y < enemy_y) 
    {
        dy = -1;
    }

    // Check if the enemy can move in the calculated direction
    float new_x = enemy_x + dx;
    float new_y = enemy_y + dy;

    // Ensure the new position is within the grid bounds
    entity->position.x = new_x;
    entity->position.y = new_y;
}
