typedef struct {
    float x;
    float y;
} Vector2f;

// Entity struct with position, velocity, and state
typedef struct
{
    Vector2f position;        // Current position
    Vector2f velocity;        // Current velocity (m/s)
    Vector2f target_position; // Target position (for pathfinding)
    float acceleration;
    float max_speed;
    int radius;
} Entity;

float computeNearestObstacleDistance(int x, 
                                     int y, 
                                     const uchar* grid, 
                                     int width, 
                                     int height,
                                     int obstacle_check_radius)
{
    float min_distance = FLT_MAX;

    for (int oy = -obstacle_check_radius; oy <= obstacle_check_radius; oy++) 
    {
        for (int ox = -obstacle_check_radius; ox <= obstacle_check_radius; ox++) 
        {
            int nx = x + ox;
            int ny = y + oy;

            if (nx < 0 || nx >= width || ny < 0 || ny >= height) 
                continue;

            // Obstacle pixel
            if (grid[nx * width + ny] < 250) 
            { 
                float distance = length((float2)(ox, oy));
                if (distance < min_distance) 
                {
                    min_distance = distance;
                }
            }
        }
    }
    return min_distance;
}

float2 ScoreTargetPosition(float2 position,
                           const uchar* grid,
                           int width,
                           int height,
                           int step,
                           int search_radius,
                           float2 goal_position)
{
    float max_distance = length((float2)(search_radius, search_radius));
    int obstacle_check_radius = 3;

    const float alpha = 2.0f;
    const float beta = 50.0f;

    bool found_position = false;
    float best_score = -FLT_MAX;
    float2 best_position = (float2)(0, 0);

    for (int y = -search_radius; y <= search_radius; y += step)
    {
        for (int x = -search_radius; x <= search_radius; x += step)
        {
            // Search only a circle
            if (length((float2)(x, y)) > search_radius)
                continue;

            int grid_x = (int)position.x + x;
            int grid_y = (int)position.y + y;

            // Check boundaries
            if (grid_x < 0 || grid_x >= width || grid_y < 0 || grid_y >= height)
                continue;

            // Check if the position is not an obstacle
            if (grid[grid_x * width + grid_y] > 250)
            {
                // Valid position: compute its score
                float2 candidate_position = (float2)(grid_x, grid_y);
                float distance_to_goal = length(candidate_position - goal_position);
                float normalized_distance_to_goal = distance_to_goal / max_distance;

                float distance_to_current = length(candidate_position - position);

                float distance_to_obstacle = computeNearestObstacleDistance(grid_x, grid_y, grid, width, height, obstacle_check_radius);
                float inverse_distance_to_obstacle = 1.0f / (1.0f + distance_to_obstacle);

                // Score calculation
                float score = -distance_to_goal + distance_to_current - (beta * inverse_distance_to_obstacle);

                // Keep track of the best position
                if (score > best_score)
                {
                    found_position = true;
                    best_score = score;
                    best_position = candidate_position;
                }
            }
        }
    }

    if (found_position)
    {
        return best_position;
    }
    return position;
}

void DynamiceHeurisitic(Entity* entity,
                        const uchar* grid,
                        int width,
                        int height,
                        float2 goal_position,
                        int search_radius)
{
    int coarseStep = 2;

    float2 currPosition;
    currPosition.x = entity->position.x;
    currPosition.y = entity->position.y;

    float2 bestCoarsePosition = ScoreTargetPosition(currPosition, grid, width, height, coarseStep, search_radius, goal_position);

    float2 finePosition = ScoreTargetPosition(bestCoarsePosition, grid, width, height, 1, coarseStep, goal_position);

    entity->target_position.x = finePosition.x;
    entity->target_position.y = finePosition.y;
}

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

    float distance_to_goal = length((float2)(entity->position.x, entity->position.y) - goal_position);
    float distance_to_target_position = length((float2)(entity->position.x, entity->position.y) - (float2)(entity->target_position.x, entity->target_position.y));
    if (distance_to_goal > entity->radius)
    {
        int search_radius = 10;

        if (distance_to_target_position < entity->radius)
        {
            DynamiceHeurisitic(entity, grid, width, height, goal_position, search_radius);
        }

        float2 direction_to_target = (float2)(entity->target_position.x - entity->position.x,
                                              entity->target_position.y - entity->position.y);

        float distance = sqrt(direction_to_target.x * direction_to_target.x + direction_to_target.y * direction_to_target.y);

        if (distance > 0.0f)
        {
            // Update entity's velocity and position smoothly
            float2 desired_velocity = normalize(direction_to_target) * entity->max_speed;
            float2 curr_velocity = (float2)(entity->velocity.x, entity->velocity.y);

            float2 new_velocity = curr_velocity + ((desired_velocity - curr_velocity) * dt * entity->acceleration);
            float2 mod_position = (new_velocity * dt);
            entity->position.x += mod_position.x;
            entity->position.y += mod_position.y;

            entity->velocity.x = new_velocity.x;
            entity->velocity.y = new_velocity.y;
        }
    }
}
