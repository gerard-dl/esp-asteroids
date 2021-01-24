//
//  RenderBuffer.cpp
//  AsteroidsTestBed
//
//  Created by Chris Greening on 08/01/2021.
//

#include "RenderBuffer.hpp"
#include "box2d/box2d.h"
#include "../Game/GameObject.hpp"
#include "../Game/Game.hpp"
#include <list>

RenderBuffer::RenderBuffer(int minX, int maxX, int minY, int maxY, int centerX, int centerY, float scale)
{
    display_frame = new std::vector<DrawInstruction_t>(500);
    drawing_frame = new std::vector<DrawInstruction_t>(500);
    _minX = minX;
    _minY = minY;
    _maxX = maxX;
    _maxY = maxY;
    _centerX = centerX;
    _centerY = centerY;
    _scale = scale;
}

// https : //easings.net/#easeInOutQuint
float easeInOutCubic(float x)
{
    return x < 0.5 ? 4 * x * x * x : 1 - powf(-2 * x + 2, 3) / 2;
}

float easeInOutSine(float x)
{
    return -(cosf(M_PI * x) - 1.0f) / 2.0f;
}

float easeInOutQuint(float x)
{
    return x < 0.5 ? 16.0f * x * x * x * x * x : 1.0f - powf(-2.0f * x + 2.0f, 5.0f) / 2.0f;
}

void RenderBuffer::renderSegment(bool laser, b2Vec2 start, const b2Vec2 &end)
{
    auto vector = end - start;
    auto length = vector.Length();
    int16_t hold = (int16_t)(std::min(10.0f, 1.5f * length)); // this is a bit of finger in the air fudge
    drawing_frame->push_back({.x = calc_x(start.x + vector.x),
                              .y = calc_y(start.y + vector.y),
                              .hold = hold,
                              .laser = laser});
}

GameObject *removeNearest(b2Vec2 search_point, std::list<GameObject *> &objects)
{
    GameObject *nearest_object = NULL;
    float nearest_distance = FLT_MAX;
    for (auto object : objects)
    {
        auto distance = (object->getPosition() - search_point).LengthSquared();
        if (distance < nearest_distance)
        {
            nearest_distance = distance;
            nearest_object = object;
        }
    }
    objects.remove(nearest_object);
    return nearest_object;
}

void RenderBuffer::render_if_needed(Game *game)
{
    if (needs_render)
    {
        drawing_frame->clear();
        // assume we start from 0,0 - we might optimise this later to start from the last drawn location
        b2Vec2 cur(0, 0);
        std::list<GameObject *> objects_to_draw(game->getObjects());
        // while we still have objects to draw
        while (objects_to_draw.size() > 0)
        {
            // get the nearest object to the current search_point
            auto object = removeNearest(cur, objects_to_draw);
            auto numPoints = object->getNumPoints();
            auto points = object->getPoints();
            auto position = object->getPosition();
            auto angle = object->getAngle();
            auto c = cos(angle);
            auto s = sin(angle);
            // move to the start of the object
            auto start = b2Vec2(points[0].x * c - points[0].y * s, points[0].x * s + points[0].y * c) + position;
            renderSegment(false, cur, start);

            cur = start;
            for (int i = 0; i < numPoints; i++)
            {
                // draw each line segment
                auto p = b2Vec2(points[i].x * c - points[i].y * s, points[i].x * s + points[i].y * c) + position;
                renderSegment(true, cur, p);
                cur = p;
            }
            // close the line
            renderSegment(true, cur, start);
            cur = start;
        }
        // move back to 0,0 ready for the next draw pass
        renderSegment(false, cur, b2Vec2(0, 0));
        // finished rendering
        needs_render = false;
    }
}

bool RenderBuffer::swapBuffers()
{
    // check to see if the other buffer is ready
    if (needs_render)
    {
        // if it's not don't swap as there's nothing to swap to!
        return false;
    }
    std::swap(display_frame, drawing_frame);
    needs_render = true;
    return true;
}
