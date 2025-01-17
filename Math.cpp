#include "Math.h"

double GMath::scalar_mult(V4 vec1, V4 vec2)
{
    double result = vec1.a * vec2.a + vec1.b * vec2.b
                    + vec1.c * vec2.c + vec1.d * vec2.d;
    return result;
}

//for normalize(V4)
V4 GMath::scalar_mult(V4 vec, double g)
{
    return {vec.a * g, vec.b * g, vec.c * g};
}

double GMath::modul(V4 vec)
{
    double result = std::sqrt(std::pow(vec.a, 2) + std::pow(vec.b, 2)
            + std::pow(vec.c, 2) + std::pow(vec.d, 2));
    return result;
}

Point GMath::polar_to_dec(double ro, double phi)
{
    Point point;
    point.rx = ro * std::cos(phi);
    point.ry = ro * std::sin(phi);
    return point;
}

V4 GMath::plane_equation(Point p1, Point p2, Point p3)
{
    // + - +
    p2 = {p2.x - p1.x, p2.y - p1.y, p2.z - p1.z};
    p3 = {p3.x - p1.x, p3.y - p1.y, p3.z - p1.z};
    p1 = {-p1.x, -p1.y, -p1.z};

    Point minors= {(p2.y * p3.z - p3.y * p2.z), (p2.x * p3.z - p3.x * p2.z), 
        (p2.x * p3.y - p3.x * p2.y)};
    V4 result = {minors.x, -minors.y, minors.z, p1.x * minors.x + (-1) * p1.y * minors.y + p1.z * 
        minors.z};
    //printf("%f, %f, %f, %f \n", result.a, result.b, result.c, result.d);
    return result;
}

std::vector <V4> GMath::get_planeset(std::vector <Point> vertex, 
        std::vector <std::vector <int>> planeset)
{
    std::vector <V4> result(planeset.size());
    for(size_t i = 0; i < planeset.size(); ++i)
    {
        result[i] = plane_equation(vertex[planeset[i][0]], vertex[planeset[i][1]], 
                vertex[planeset[i][2]]);
    }
    return result;
}

//in 2d
double GMath::dist_flat(Point p1, Point p2)
{
    double a = std::abs(p2.x - p1.x);
    double b = std::abs(p2.y - p1.y);
    double result = std::sqrt(std::pow(a, 2) + std::pow(b, 2));
    return result;
}

//in 3d
double GMath::dist_stereo(Point p1, Point p2)
{
    double a = std::abs(p2.x - p1.x);
    double b = std::abs(p2.y - p1.y);
    double c = std::abs(p2.z - p1.z);
    double result = std::sqrt(std::pow(a, 2) + std::pow(b, 2) + std::pow(c, 2));
    return result;
}

V4 GMath::normalize(V4 vec)
{
    double inv_length = 1.0 / std::sqrt(std::pow(vec.a, 2) + std::pow(vec.b, 2) + std::pow(vec.c, 2))
        + std::pow(vec.d, 2);
    return scalar_mult(vec, inv_length);
}

Point GMath::real_point(Point origin, Point a)
{   
    Point point = {origin.x + a.x, origin.y - a.y, origin.z + a.z};
    return point;
}

//screen center
Point GMath::find_origin(int size_x, int size_y, double k)
{
    Point point = {std::round(size_x / 2), std::round(size_y / 2), k};
    return point;
}

std::vector <Edge> GMath::edges_to_render(std::vector <V4> planes, 
        std::vector <std::vector <int>> connections, std::vector <Point> obj)
{
    std::vector <Edge> result;
    for(size_t i = 0; i < planes.size(); ++i)
    {
        for(size_t j = 0; j < obj.size(); ++j)
        {
            // if point belongs to plane ...
            V4 v = {obj[j].x, obj[j].y, obj[j].z, 1};
            if(std::abs(std::round(scalar_mult(v, planes[i])*1000)/1000) == 0.000)
            {
                for(size_t t = 0; t < connections[j].size(); ++t)
                {
                    V4 v1 = {obj[connections[j][t]].x, obj[connections[j][t]].y, 
                        obj[connections[j][t]].z, 1};
                    if(std::abs(std::round(scalar_mult(v1, planes[i])*1000)/1000) == 0.000)
                    {
                        Edge edges = {obj[j], obj[connections[j][t]]};
                        result.push_back(edges);
                    }
                }
            }
        }
    }
    return result;
}

// here we take visible sides (equations of planes) 
// and mesure brightness (from max -> min)
std::vector <double> GMath::brightness(std::vector <V4> planes, 
        std::vector <std::vector <Point>> tri_out, V4 light)
{
    std::vector <double> result(tri_out.size());
    double max = -1;

    bool flag = true; // means, that all 3 verticies are on current plane
    for(size_t i = 0; i < planes.size(); ++i)
    {
        for(size_t j = 0; j < tri_out.size(); ++j)
        {
            for(size_t t = 0; t < tri_out[j].size(); ++t) // t = {0 .. 2}
            {
                // if point belongs to plane ...
                V4 v = {tri_out[j][t].x, tri_out[j][t].y, tri_out[j][t].z, 1};
                if(std::abs(std::round(scalar_mult(v, planes[i])*1000)/1000) != 0.000)
                {
                    flag = false; 
                }
            }
            if(flag == true)
            {
                result[j] = scalar_mult(light, normalize(planes[i])) / (modul(light)
                    * modul(normalize(planes[i]))) * 100;

                if(result[j] > max)
                {
                    max = result[j];
                }
                if(result[j] < 0)
                    result[j] = 0;
            }
            else
            {
                flag = true;
            }
        }
    }
    return result;
}

std::vector <std::vector <Point>> GMath::tri_to_render(std::vector <V4> planes, 
        std::vector <std::vector <int>> tri, std::vector <Point> obj)
{
    std::vector <std::vector <Point>> result;
    bool flag = true; // green light to push_back in result
    for(size_t i = 0; i < planes.size(); ++i)
    {
        for(size_t j = 0; j < tri.size(); ++j)
        {
            std::vector <Point> tmp = {};
            for(int t = 0; t < tri[j].size(); ++t) // t = {0 .. 2}
            {
                // if point belongs to plane ...
                V4 v = {obj[tri[j][t]].x, obj[tri[j][t]].y, obj[tri[j][t]].z, 1};
                if(std::abs(std::round(scalar_mult(v, planes[i])*1000)/1000) == 0.000)
                {
                    tmp.push_back(obj[tri[j][t]]); 
                }
                else
                {
                    flag = false; // ur unable to push in result
                }
            }
            if(flag == true)
            {
                result.push_back(tmp);
            }
            else
            {
                tmp = {};
                flag = true;
            }
        }
    }
    return result;
}

std::vector <V4> GMath::visibility(std::vector <V4> list, V4 camera)
{
    std::vector <V4> result;
    for(size_t i = 0; i < list.size(); ++i)
    {
        /* change for clockwise or counterclockwise systems! */
         
        if(scalar_mult(camera, list[i]) > 0)
        {
            result.push_back(list[i]);
        }
    }
    return result;
}

void GMath::get_sides(std::vector <std::vector <int>> &sides, 
        std::vector <std::vector <int>> planeset, std::vector <Point> vertex)
{
    std::vector <V4> planes = GMath::get_planeset(vertex, planeset);
    sides.resize(planeset.size());
    for(size_t i = 0; i < planes.size(); ++i)
    {
        for(size_t j = 0; j < vertex.size(); ++j)
        {
            // if point belongs to plane ...
            V4 v = {vertex[j].x, vertex[j].y, vertex[j].z, 1};
            if(std::abs(std::round(GMath::scalar_mult(v, planes[i])*1000)/1000) == 0.000)
            {
                sides[i].push_back(j);
            }
        }
    }
}

void GMath::convex_hull(std::vector <std::vector <int>> &sides, 
        std::vector <std::vector <int>> connections)
{
    /* here 'll process sides to make them as "list" by connections */
    std::vector <std::vector <int>> new_sides;
    new_sides.resize(sides.size());
    /* for all sides */
    for(size_t i = 0; i < sides.size(); ++i) 
    {
        new_sides[i].push_back(sides[i][0]);
        bool flag_root = false;
        bool close_flag = false;

        /* prev - back - <- upcoming */
        int prev = sides[i][0];
        int close = -1; 

    endof:
        if(new_sides[i].back() != close || flag_root == false)
        {
            /* for all con's for elem on side */
            for(size_t m = 0; m < connections[new_sides[i].back()].size(); ++m)
            {
                /* for all elems on side */
                for(size_t l = 0; l < sides[i].size(); ++l)
                {
                    /* first one "good" leaf */
                    if(sides[i][l] == connections[new_sides[i].back()][m] && flag_root == false)
                    {
                        // its first neighbor
                        if(close_flag == false)
                        {
                            close = sides[i][l];
                            close_flag = true;
                        }
                        else
                        {
                            flag_root = true;
                            new_sides[i].push_back(sides[i][l]);
                            goto endof;
                        } 
                    }
                    else
                    {
                        if(sides[i][l] == connections[new_sides[i].back()][m] && sides[i][l] != prev)
                        {
                            prev = new_sides[i].back();
                            new_sides[i].push_back(sides[i][l]);
                            goto endof;
                        }
                    }
                }
            }
        }
    }
    sides = new_sides;
}

/* works only for convex hull!! */
std::vector <std::vector <int>> GMath::gypsy_delon(std::vector <std::vector <int>> sides)
{
    std::vector <std::vector <int>> result;
    for(int i = 0; i < (int)sides.size(); ++i)
    {
        // number of triangles in poly (n - 2), where n -- number of vertecies
        for(int j = 2; j < (int)sides[i].size(); ++j)
        { 
            result.push_back({sides[i][0], sides[i][j - 1], sides[i][j]});
        }
    }
    return result;
}
