//
// Created by yongfali on 2016/4/21.
//
#ifndef E_VECTOR_H
#define E_VECTOR_H

#include <cmath>

namespace e
{
    ///////////////////////////////////////////////////////////////////////////////
    // 2D vector
    ///////////////////////////////////////////////////////////////////////////////
    struct Vector2
    {
        float x;
        float y;

        // ctors
        Vector2() : x(0), y(0) {};
        Vector2(float x, float y) : x(x), y(y) {};

        // utils functions
        void        Set(float x, float y);
        float       Length() const;                         //
        float       Distance(const Vector2& vec) const;     // distance between two vectors
        Vector2&    Normalize();                            //
        float       Dot(const Vector2& vec) const;          // dot product
        bool        Equal(const Vector2& vec, float e) const; // compare with epsilon

        // operators
        Vector2     operator-() const;                      // unary operator (negate)
        Vector2     operator+(const Vector2& rhs) const;    // add rhs
        Vector2     operator-(const Vector2& rhs) const;    // subtract rhs
        Vector2&    operator+=(const Vector2& rhs);         // add rhs and update this object
        Vector2&    operator-=(const Vector2& rhs);         // subtract rhs and update this object
        Vector2     operator*(const float scale) const;     // scale
        Vector2     operator*(const Vector2& rhs) const;    // multiply each element
        Vector2&    operator*=(const float scale);          // scale and update this object
        Vector2&    operator*=(const Vector2& rhs);         // multiply each element and update this object
        Vector2     operator/(const float scale) const;     // inverse scale
        Vector2&    operator/=(const float scale);          // scale and update this object
        bool        operator==(const Vector2& rhs) const;   // exact compare, no epsilon
        bool        operator!=(const Vector2& rhs) const;   // exact compare, no epsilon
        bool        operator<(const Vector2& rhs) const;    // comparison for sort
        float       operator[](int index) const;            // subscript operator v[0], v[1]
        float&      operator[](int index);                  // subscript operator v[0], v[1]

        friend Vector2 operator*(const float a, const Vector2 vec);
    };



    ///////////////////////////////////////////////////////////////////////////////
    // 3D vector
    ///////////////////////////////////////////////////////////////////////////////
    struct Vector3
    {
        float x;
        float y;
        float z;

        // ctors
        Vector3() : x(0), y(0), z(0) {};
        Vector3(float x, float y, float z) : x(x), y(y), z(z) {};

        // utils functions
        void        Set(float x, float y, float z);
        float       Length() const;                         //
        float       Distance(const Vector3& vec) const;     // distance between two vectors
        Vector3&    Normalize();                            //
        float       Dot(const Vector3& vec) const;          // dot product
        Vector3     Cross(const Vector3& vec) const;        // cross product
        bool        Equal(const Vector3& vec, float e) const; // compare with epsilon

        // operators
        Vector3     operator-() const;                      // unary operator (negate)
        Vector3     operator+(const Vector3& rhs) const;    // add rhs
        Vector3     operator-(const Vector3& rhs) const;    // subtract rhs
        Vector3&    operator+=(const Vector3& rhs);         // add rhs and update this object
        Vector3&    operator-=(const Vector3& rhs);         // subtract rhs and update this object
        Vector3     operator*(const float scale) const;     // scale
        Vector3     operator*(const Vector3& rhs) const;    // multiplay each element
        Vector3&    operator*=(const float scale);          // scale and update this object
        Vector3&    operator*=(const Vector3& rhs);         // product each element and update this object
        Vector3     operator/(const float scale) const;     // inverse scale
        Vector3&    operator/=(const float scale);          // scale and update this object
        bool        operator==(const Vector3& rhs) const;   // exact compare, no epsilon
        bool        operator!=(const Vector3& rhs) const;   // exact compare, no epsilon
        bool        operator<(const Vector3& rhs) const;    // comparison for sort
        float       operator[](int index) const;            // subscript operator v[0], v[1]
        float&      operator[](int index);                  // subscript operator v[0], v[1]

        friend Vector3 operator*(const float a, const Vector3 vec);
    };



    ///////////////////////////////////////////////////////////////////////////////
    // 4D vector
    ///////////////////////////////////////////////////////////////////////////////
    struct Vector4
    {
        float x;
        float y;
        float z;
        float w;

        // ctors
        Vector4() : x(0), y(0), z(0), w(0) {};
        Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {};

        // utils functions
        void        Set(float x, float y, float z, float w);
        float       Length() const;                         //
        float       Distance(const Vector4& vec) const;     // distance between two vectors
        Vector4&    Normalize();                            //
        float       Dot(const Vector4& vec) const;          // dot product
        bool        Equal(const Vector4& vec, float e) const; // compare with epsilon

        // operators
        Vector4     operator-() const;                      // unary operator (negate)
        Vector4     operator+(const Vector4& rhs) const;    // add rhs
        Vector4     operator-(const Vector4& rhs) const;    // subtract rhs
        Vector4&    operator+=(const Vector4& rhs);         // add rhs and update this object
        Vector4&    operator-=(const Vector4& rhs);         // subtract rhs and update this object
        Vector4     operator*(const float scale) const;     // scale
        Vector4     operator*(const Vector4& rhs) const;    // multiply each element
        Vector4&    operator*=(const float scale);          // scale and update this object
        Vector4&    operator*=(const Vector4& rhs);         // multiply each element and update this object
        Vector4     operator/(const float scale) const;     // inverse scale
        Vector4&    operator/=(const float scale);          // scale and update this object
        bool        operator==(const Vector4& rhs) const;   // exact compare, no epsilon
        bool        operator!=(const Vector4& rhs) const;   // exact compare, no epsilon
        bool        operator<(const Vector4& rhs) const;    // comparison for sort
        float       operator[](int index) const;            // subscript operator v[0], v[1]
        float&      operator[](int index);                  // subscript operator v[0], v[1]

        friend Vector4 operator*(const float a, const Vector4 vec);
    };



    // fast math routines from Doom3 SDK
    // inline float InvSqrt(float x)
    // {
    //     float xhalf = 0.5f * x;
    //     int i = *(int*)&x;          // get bits for floating value
    //     i = 0x5f3759df - (i>>1);    // gives initial guess
    //     x = *((float*)&i);          // convert bits back to float
    //     x = x * (1.5f - xhalf*x*x); // Newton step
    //     return x;
    // }

    ///////////////////////////////////////////////////////////////////////////////
    // inline functions for Vector2
    ///////////////////////////////////////////////////////////////////////////////
    inline Vector2 Vector2::operator-() const 
    {
        return Vector2(-x, -y);
    }

    inline Vector2 Vector2::operator+(const Vector2& rhs) const 
    {
        return Vector2(x+rhs.x, y+rhs.y);
    }

    inline Vector2 Vector2::operator-(const Vector2& rhs) const 
    {
        return Vector2(x-rhs.x, y-rhs.y);
    }

    inline Vector2& Vector2::operator+=(const Vector2& rhs) 
    {
        x += rhs.x; y += rhs.y; return *this;
    }

    inline Vector2& Vector2::operator-=(const Vector2& rhs) 
    {
        x -= rhs.x; y -= rhs.y; return *this;
    }

    inline Vector2 Vector2::operator*(const float a) const 
    {
        return Vector2(x*a, y*a);
    }

    inline Vector2 Vector2::operator*(const Vector2& rhs) const 
    {
        return Vector2(x*rhs.x, y*rhs.y);
    }

    inline Vector2& Vector2::operator*=(const float a) 
    {
        x *= a; y *= a; return *this;
    }

    inline Vector2& Vector2::operator*=(const Vector2& rhs) 
    {
        x *= rhs.x; y *= rhs.y; return *this;
    }

    inline Vector2 Vector2::operator/(const float a) const 
    {
        return Vector2(x/a, y/a);
    }

    inline Vector2& Vector2::operator/=(const float a) 
    {
        x /= a; y /= a; return *this;
    }

    inline bool Vector2::operator==(const Vector2& rhs) const
     {
        return (x == rhs.x) && (y == rhs.y);
    }

    inline bool Vector2::operator!=(const Vector2& rhs) const 
    {
        return (x != rhs.x) || (y != rhs.y);
    }

    inline bool Vector2::operator<(const Vector2& rhs) const 
    {
        if(x < rhs.x) return true;
        if(x > rhs.x) return false;
        if(y < rhs.y) return true;
        if(y > rhs.y) return false;
        return false;
    }

    inline float Vector2::operator[](int index) const 
    {
        return (&x)[index];
    }

    inline float& Vector2::operator[](int index) 
    {
        return (&x)[index];
    }

    inline void Vector2::Set(float x, float y) 
    {
        this->x = x; this->y = y;
    }

    inline float Vector2::Length() const 
    {
        return sqrtf(x*x + y*y);
    }

    inline float Vector2::Distance(const Vector2& vec) const 
    {
        return sqrtf((vec.x-x)*(vec.x-x) + (vec.y-y)*(vec.y-y));
    }

    inline Vector2& Vector2::Normalize() 
    {
        //@@const float EPSILON = 0.000001f;
        float xxyy = x*x + y*y;
        //@@if(xxyy < EPSILON)
        //@@    return *this;

        //float invLength = InvSqrt(xxyy);
        float invLength = 1.0f / sqrtf(xxyy);
        x *= invLength;
        y *= invLength;
        return *this;
    }

    inline float Vector2::Dot(const Vector2& rhs) const 
    {
        return (x*rhs.x + y*rhs.y);
    }

    inline bool Vector2::Equal(const Vector2& rhs, float epsilon) const 
    {
        return fabs(x - rhs.x) < epsilon && fabs(y - rhs.y) < epsilon;
    }

    inline Vector2 operator*(const float a, const Vector2 vec) 
    {
        return Vector2(a*vec.x, a*vec.y);
    }

    // END OF VECTOR2 /////////////////////////////////////////////////////////////




    ///////////////////////////////////////////////////////////////////////////////
    // inline functions for Vector3
    ///////////////////////////////////////////////////////////////////////////////
    inline Vector3 Vector3::operator-() const 
    {
        return Vector3(-x, -y, -z);
    }

    inline Vector3 Vector3::operator+(const Vector3& rhs) const 
    {
        return Vector3(x+rhs.x, y+rhs.y, z+rhs.z);
    }

    inline Vector3 Vector3::operator-(const Vector3& rhs) const
     {
        return Vector3(x-rhs.x, y-rhs.y, z-rhs.z);
    }

    inline Vector3& Vector3::operator+=(const Vector3& rhs) 
    {
        x += rhs.x; y += rhs.y; z += rhs.z; return *this;
    }

    inline Vector3& Vector3::operator-=(const Vector3& rhs) 
    {
        x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this;
    }

    inline Vector3 Vector3::operator*(const float a) const 
    {
        return Vector3(x*a, y*a, z*a);
    }

    inline Vector3 Vector3::operator*(const Vector3& rhs) const 
    {
        return Vector3(x*rhs.x, y*rhs.y, z*rhs.z);
    }

    inline Vector3& Vector3::operator*=(const float a)
    {
        x *= a; y *= a; z *= a; return *this;
    }

    inline Vector3& Vector3::operator*=(const Vector3& rhs) 
    {
        x *= rhs.x; y *= rhs.y; z *= rhs.z; return *this;
    }

    inline Vector3 Vector3::operator/(const float a) const 
    {
        return Vector3(x/a, y/a, z/a);
    }

    inline Vector3& Vector3::operator/=(const float a) 
    {
        x /= a; y /= a; z /= a; return *this;
    }

    inline bool Vector3::operator==(const Vector3& rhs) const 
    {
        return (x == rhs.x) && (y == rhs.y) && (z == rhs.z);
    }

    inline bool Vector3::operator!=(const Vector3& rhs) const 
    {
        return (x != rhs.x) || (y != rhs.y) || (z != rhs.z);
    }

    inline bool Vector3::operator<(const Vector3& rhs) const 
    {
        if(x < rhs.x) return true;
        if(x > rhs.x) return false;
        if(y < rhs.y) return true;
        if(y > rhs.y) return false;
        if(z < rhs.z) return true;
        if(z > rhs.z) return false;
        return false;
    }

    inline float Vector3::operator[](int index) const 
    {
        return (&x)[index];
    }

    inline float& Vector3::operator[](int index) 
    {
        return (&x)[index];
    }

    inline void Vector3::Set(float x, float y, float z) 
    {
        this->x = x; this->y = y; this->z = z;
    }

    inline float Vector3::Length() const {
        return sqrtf(x*x + y*y + z*z);
    }

    inline float Vector3::Distance(const Vector3& vec) const 
    {
        return sqrtf((vec.x-x)*(vec.x-x) + (vec.y-y)*(vec.y-y) + (vec.z-z)*(vec.z-z));
    }

    inline Vector3& Vector3::Normalize() 
    {
        //@@const float EPSILON = 0.000001f;
        float xxyyzz = x*x + y*y + z*z;
        //@@if(xxyyzz < EPSILON)
        //@@    return *this; // do nothing if it is ~zero vector

        //float invLength = InvSqrt(xxyyzz);
        float invLength = 1.0f / sqrtf(xxyyzz);
        x *= invLength;
        y *= invLength;
        z *= invLength;
        return *this;
    }

    inline float Vector3::Dot(const Vector3& rhs) const 
    {
        return (x*rhs.x + y*rhs.y + z*rhs.z);
    }

    inline Vector3 Vector3::Cross(const Vector3& rhs) const 
    {
        return Vector3(y*rhs.z - z*rhs.y, z*rhs.x - x*rhs.z, x*rhs.y - y*rhs.x);
    }

    inline bool Vector3::Equal(const Vector3& rhs, float epsilon) const 
    {
        return fabs(x - rhs.x) < epsilon && fabs(y - rhs.y) < epsilon && fabs(z - rhs.z) < epsilon;
    }

    inline Vector3 operator*(const float a, const Vector3 vec) 
    {
        return Vector3(a*vec.x, a*vec.y, a*vec.z);
    }

    // END OF VECTOR3 /////////////////////////////////////////////////////////////



    ///////////////////////////////////////////////////////////////////////////////
    // inline functions for Vector4
    ///////////////////////////////////////////////////////////////////////////////
    inline Vector4 Vector4::operator-() const 
    {
        return Vector4(-x, -y, -z, -w);
    }

    inline Vector4 Vector4::operator+(const Vector4& rhs) const 
    {
        return Vector4(x+rhs.x, y+rhs.y, z+rhs.z, w+rhs.w);
    }

    inline Vector4 Vector4::operator-(const Vector4& rhs) const 
    {
        return Vector4(x-rhs.x, y-rhs.y, z-rhs.z, w-rhs.w);
    }

    inline Vector4& Vector4::operator+=(const Vector4& rhs) 
    {
        x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this;
    }

    inline Vector4& Vector4::operator-=(const Vector4& rhs) 
    {
        x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this;
    }

    inline Vector4 Vector4::operator*(const float a) const 
    {
        return Vector4(x*a, y*a, z*a, w*a);
    }

    inline Vector4 Vector4::operator*(const Vector4& rhs) const 
    {
        return Vector4(x*rhs.x, y*rhs.y, z*rhs.z, w*rhs.w);
    }

    inline Vector4& Vector4::operator*=(const float a) 
    {
        x *= a; y *= a; z *= a; w *= a; return *this;
    }

    inline Vector4& Vector4::operator*=(const Vector4& rhs) 
    {
        x *= rhs.x; y *= rhs.y; z *= rhs.z; w *= rhs.w; return *this;
    }

    inline Vector4 Vector4::operator/(const float a) const 
    {
        return Vector4(x/a, y/a, z/a, w/a);
    }

    inline Vector4& Vector4::operator/=(const float a) 
    {
        x /= a; y /= a; z /= a; w /= a; return *this;
    }

    inline bool Vector4::operator==(const Vector4& rhs) const 
    {
        return (x == rhs.x) && (y == rhs.y) && (z == rhs.z) && (w == rhs.w);
    }

    inline bool Vector4::operator!=(const Vector4& rhs) const 
    {
        return (x != rhs.x) || (y != rhs.y) || (z != rhs.z) || (w != rhs.w);
    }

    inline bool Vector4::operator<(const Vector4& rhs) const 
    {
        if(x < rhs.x) return true;
        if(x > rhs.x) return false;
        if(y < rhs.y) return true;
        if(y > rhs.y) return false;
        if(z < rhs.z) return true;
        if(z > rhs.z) return false;
        if(w < rhs.w) return true;
        if(w > rhs.w) return false;
        return false;
    }

    inline float Vector4::operator[](int index) const 
    {
        return (&x)[index];
    }

    inline float& Vector4::operator[](int index) 
    {
        return (&x)[index];
    }

    inline void Vector4::Set(float x, float y, float z, float w) 
    {
        this->x = x; this->y = y; this->z = z; this->w = w;
    }

    inline float Vector4::Length() const 
    {
        return sqrtf(x*x + y*y + z*z + w*w);
    }

    inline float Vector4::Distance(const Vector4& vec) const 
    {
        return sqrtf((vec.x-x)*(vec.x-x) + (vec.y-y)*(vec.y-y) + (vec.z-z)*(vec.z-z) + (vec.w-w)*(vec.w-w));
    }

    inline Vector4& Vector4::Normalize() 
    {
        //NOTE: leave w-component untouched
        //@@const float EPSILON = 0.000001f;
        float xxyyzz = x*x + y*y + z*z;
        //@@if(xxyyzz < EPSILON)
        //@@    return *this; // do nothing if it is zero vector

        //float invLength = InvSqrt(xxyyzz);
        float invLength = 1.0f / sqrtf(xxyyzz);
        x *= invLength;
        y *= invLength;
        z *= invLength;
        return *this;
    }

    inline float Vector4::Dot(const Vector4& rhs) const 
    {
        return (x*rhs.x + y*rhs.y + z*rhs.z + w*rhs.w);
    }

    inline bool Vector4::Equal(const Vector4& rhs, float epsilon) const 
    {
        return fabs(x - rhs.x) < epsilon && fabs(y - rhs.y) < epsilon &&
               fabs(z - rhs.z) < epsilon && fabs(w - rhs.w) < epsilon;
    }

    inline Vector4 operator*(const float a, const Vector4 vec)
    {
        return Vector4(a*vec.x, a*vec.y, a*vec.z, a*vec.w);
    }

}
#endif
