#ifndef BVH_H
#define BVH_H

#include "hittable.h"
#include "hittable_list.h"
#include "utility.h"

#include <algorithm>

// Class for bounding volume hierarchies. It constructs the bvh tree upon initialization using middle point method.
// Reference: Ray Tracing: The Next Week, Physically Based Rendering
class bvh_node : public hittable {
public:
    bvh_node();

    bvh_node(const hittable_list& list, double time0, double time1)
        : bvh_node(list.objects, 0, list.objects.size(), time0, time1, 0)
    {}

    bvh_node(
        const std::vector<shared_ptr<hittable>>& src_objects,
        size_t start, size_t end, double time0, double time1, int depth);

    virtual bool hit(
        const ray& r, double t_min, double t_max, hit_record& rec) const override;

    virtual bool bounding_box(double time0, double time1, aabb& output_box) const override;

public:
    shared_ptr<hittable> left;
    shared_ptr<hittable> right;
    aabb box;
};

// Traverse through the bvh tree to find the hit point and update hit record
bool bvh_node::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    // If ray does not hit the box, it must not hit its children
    if (!box.hit(r, t_min, t_max))
        return false;

    // Traverse through children to check for hit
    bool hit_left = left->hit(r, t_min, t_max, rec);
    bool hit_right = right->hit(r, t_min, hit_left ? rec.t : t_max, rec);

    return hit_left || hit_right;
}

bool bvh_node::bounding_box(double time0, double time1, aabb& output_box) const {
    output_box = box;
    return true;
}

// Construct bvh tree upon initialization
bvh_node::bvh_node(
    const std::vector<shared_ptr<hittable>>& src_objects,
        size_t start, size_t end, double time0, double time1, int depth) 
{
    // Create a modifiable array of the source scene objects
    std::vector<shared_ptr<hittable>> objects = src_objects;

    int axis = 0;
    double max_range = 0;
    double middle_point = 0;
    // Iterate through all the bounding boxes in the current root
    for (int i = 0; i < 3; i++) {
        double cen_i_min = db_max;
        double cen_i_max = -db_max;
        for (int j = start; j < end; j++) {
            aabb temp_box;
            if (objects[j]->bounding_box(time0, time1, temp_box)) {
                if (temp_box.cen().e[i] < cen_i_min) cen_i_min = temp_box.cen().e[i];
                if (temp_box.cen().e[i] > cen_i_max) cen_i_max = temp_box.cen().e[i];
            }
        }
        // Find that largest gap between the centroids among the axis and set it as separating axis
        // Update middle point for partition algorithm
        if (cen_i_max - cen_i_min > max_range) {
            max_range = cen_i_max - cen_i_min;
            middle_point = (cen_i_max + cen_i_min) / 2;
            axis = i;
        }
    }

    size_t object_span = end - start;

    // If 1 box in current root, set both leaf node to the object
    if (object_span == 1) {
        left = right = objects[start];
    }

    // If 2 boxes in current root, each leaf node contains 1 object
    else if (object_span == 2) {
        left = objects[start];
        right = objects[start + 1];
    }
    // If more than 2 boxes in current root, partition based on middle point
    else {
        // Find iterator among the object vector that partition based on middle point
        auto it = std::partition(objects.begin() + start, objects.begin() + end, 
            [axis, middle_point](const shared_ptr<hittable> a) {
                aabb box_a;
                if (a->bounding_box(0, 1, box_a))
                    return box_a.cen().e[axis] < middle_point;
            });

        // Recursively construct children of bvh tree
        auto mid = it - objects.begin();
        left = make_shared<bvh_node>(objects, start, mid, time0, time1, depth + 1);
        right = make_shared<bvh_node>(objects, mid, end, time0, time1, depth + 1);
    }

    aabb box_left, box_right;

    if (!left->bounding_box(time0, time1, box_left)
        || !right->bounding_box(time0, time1, box_right)
        )
        std::cerr << "No bounding box in bvh_node constructor.\n";

    // Box the current bvh root contains boxes of both children
    box = surrounding_box(box_left, box_right);
}

#endif