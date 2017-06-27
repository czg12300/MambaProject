//
// Created by yongfali on 2016/4/21.
//
#ifndef E_MATRIX_H
#define E_MATRIX_H

///////////////////////////////////////////////////////////////////////////////
//
// NxN Matrix Math classes
//
// The elements of the matrix are stored as column major order.
// | 0 2 |    | 0 3 6 |    |  0  4  8 12 |
// | 1 3 |    | 1 4 7 |    |  1  5  9 13 |
//            | 2 5 8 |    |  2  6 10 14 |
//                         |  3  7 11 15 |
///////////////////////////////////////////////////////////////////////////////
#include "Vectors.h"

namespace e
{
	///////////////////////////////////////////////////////////////////////////
	// 2x2 matrix
	///////////////////////////////////////////////////////////////////////////
	class Matrix2
	{
	public:
	    // constructors
	    Matrix2();  // init with identity
	    Matrix2(const float src[4]);
	    Matrix2(float m0, float m1, float m2, float m3);

	    void        Set(const float src[4]);
	    void        Set(float m0, float m1, float m2, float m3);
	    void        SetRow(int index, const float row[2]);
	    void        SetRow(int index, const Vector2& v);
	    void        SetColumn(int index, const float col[2]);
	    void        SetColumn(int index, const Vector2& v);

	    const float* Get() const;
	    float       GetDeterminant();

	    Matrix2&    Identity();
	    Matrix2&    Transpose();                            // transpose itself and return reference
	    Matrix2&    Invert();

	    // operators
	    Matrix2     operator+(const Matrix2& rhs) const;    // add rhs
	    Matrix2     operator-(const Matrix2& rhs) const;    // subtract rhs
	    Matrix2&    operator+=(const Matrix2& rhs);         // add rhs and update this object
	    Matrix2&    operator-=(const Matrix2& rhs);         // subtract rhs and update this object
	    Vector2     operator*(const Vector2& rhs) const;    // multiplication: v' = M * v
	    Matrix2     operator*(const Matrix2& rhs) const;    // multiplication: M3 = M1 * M2
	    Matrix2&    operator*=(const Matrix2& rhs);         // multiplication: M1' = M1 * M2
	    bool        operator==(const Matrix2& rhs) const;   // exact compare, no epsilon
	    bool        operator!=(const Matrix2& rhs) const;   // exact compare, no epsilon
	    float       operator[](int index) const;            // subscript operator v[0], v[1]
	    float&      operator[](int index);                  // subscript operator v[0], v[1]

	    friend Matrix2 operator-(const Matrix2& m);                     // unary operator (-)
	    friend Matrix2 operator*(float scalar, const Matrix2& m);       // pre-multiplication
	    friend Vector2 operator*(const Vector2& vec, const Matrix2& m); // pre-multiplication
	private:
	    float m[4];
	};



	///////////////////////////////////////////////////////////////////////////
	// 3x3 matrix
	///////////////////////////////////////////////////////////////////////////
	class Matrix3
	{
	public:
	    // constructors
	    Matrix3();  // init with identity
	    Matrix3(const float src[9]);
	    Matrix3(float m0, float m1, float m2,           // 1st column
	            float m3, float m4, float m5,           // 2nd column
	            float m6, float m7, float m8);          // 3rd column

	    void        Set(const float src[9]);
	    void        Set(float m0, float m1, float m2,   // 1st column
	                    float m3, float m4, float m5,   // 2nd column
	                    float m6, float m7, float m8);  // 3rd column
	    void        SetRow(int index, const float row[3]);
	    void        SetRow(int index, const Vector3& v);
	    void        SetColumn(int index, const float col[3]);
	    void        SetColumn(int index, const Vector3& v);

	    const float* Get() const;
	    float       GetDeterminant();

	    Matrix3&    Identity();
	    Matrix3&    Transpose();                            // transpose itself and return reference
	    Matrix3&    Invert();

	    // operators
	    Matrix3     operator+(const Matrix3& rhs) const;    // add rhs
	    Matrix3     operator-(const Matrix3& rhs) const;    // subtract rhs
	    Matrix3&    operator+=(const Matrix3& rhs);         // add rhs and update this object
	    Matrix3&    operator-=(const Matrix3& rhs);         // subtract rhs and update this object
	    Vector3     operator*(const Vector3& rhs) const;    // multiplication: v' = M * v
	    Matrix3     operator*(const Matrix3& rhs) const;    // multiplication: M3 = M1 * M2
	    Matrix3&    operator*=(const Matrix3& rhs);         // multiplication: M1' = M1 * M2
	    bool        operator==(const Matrix3& rhs) const;   // exact compare, no epsilon
	    bool        operator!=(const Matrix3& rhs) const;   // exact compare, no epsilon
	    float       operator[](int index) const;            // subscript operator v[0], v[1]
	    float&      operator[](int index);                  // subscript operator v[0], v[1]

	    friend Matrix3 operator-(const Matrix3& m);                     // unary operator (-)
	    friend Matrix3 operator*(float scalar, const Matrix3& m);       // pre-multiplication
	    friend Vector3 operator*(const Vector3& vec, const Matrix3& m); // pre-multiplication
	private:
	    float m[9];
	};



	///////////////////////////////////////////////////////////////////////////
	// 4x4 matrix
	///////////////////////////////////////////////////////////////////////////
	class Matrix4
	{
	public:
	    // constructors
	    Matrix4();  // init with identity
	    Matrix4(const float src[16]);
	    Matrix4(float m00, float m01, float m02, float m03, // 1st column
	            float m04, float m05, float m06, float m07, // 2nd column
	            float m08, float m09, float m10, float m11, // 3rd column
	            float m12, float m13, float m14, float m15);// 4th column

	    void        Set(const float src[16]);
	    void        Set(float m00, float m01, float m02, float m03, // 1st column
	                    float m04, float m05, float m06, float m07, // 2nd column
	                    float m08, float m09, float m10, float m11, // 3rd column
	                    float m12, float m13, float m14, float m15);// 4th column
	    void        SetRow(int index, const float row[4]);
	    void        SetRow(int index, const Vector4& v);
	    void        SetRow(int index, const Vector3& v);
	    void        SetColumn(int index, const float col[4]);
	    void        SetColumn(int index, const Vector4& v);
	    void        SetColumn(int index, const Vector3& v);

	    const float* Get() const;
	    const float* GetTranspose();                        // return transposed matrix
	    float        GetDeterminant();

	    Matrix4&    Identity();
	    Matrix4&    Transpose();                            // transpose itself and return reference
	    Matrix4&    Invert();                               // check best inverse method before inverse
	    Matrix4&    InvertEuclidean();                      // inverse of Euclidean transform matrix
	    Matrix4&    InvertAffine();                         // inverse of affine transform matrix
	    Matrix4&    InvertProjective();                     // inverse of projective matrix using partitioning
	    Matrix4&    InvertGeneral();                        // inverse of generic matrix

	    // transform matrix
	    Matrix4&    Translate(float x, float y, float z);   // translation by (x,y,z)
	    Matrix4&    Translate(const Vector3& v);            //
	    Matrix4&    Rotate(float angle, const Vector3& axis); // rotate angle(degree) along the given axix
	    Matrix4&    Rotate(float angle, float x, float y, float z);
	    Matrix4&    RotateX(float angle);                   // rotate on X-axis with degree
	    Matrix4&    RotateY(float angle);                   // rotate on Y-axis with degree
	    Matrix4&    RotateZ(float angle);                   // rotate on Z-axis with degree
	    Matrix4&    Scale(float scale);                     // uniform scale
	    Matrix4&    Scale(float sx, float sy, float sz);    // scale by (sx, sy, sz) on each axis

	    // operators
	    Matrix4     operator+(const Matrix4& rhs) const;    // add rhs
	    Matrix4     operator-(const Matrix4& rhs) const;    // subtract rhs
	    Matrix4&    operator+=(const Matrix4& rhs);         // add rhs and update this object
	    Matrix4&    operator-=(const Matrix4& rhs);         // subtract rhs and update this object
	    Vector4     operator*(const Vector4& rhs) const;    // multiplication: v' = M * v
	    Vector3     operator*(const Vector3& rhs) const;    // multiplication: v' = M * v
	    Matrix4     operator*(const Matrix4& rhs) const;    // multiplication: M3 = M1 * M2
	    Matrix4&    operator*=(const Matrix4& rhs);         // multiplication: M1' = M1 * M2
	    bool        operator==(const Matrix4& rhs) const;   // exact compare, no epsilon
	    bool        operator!=(const Matrix4& rhs) const;   // exact compare, no epsilon
	    float       operator[](int index) const;            // subscript operator v[0], v[1]
	    float&      operator[](int index);                  // subscript operator v[0], v[1]

	    friend Matrix4 operator-(const Matrix4& m);                     // unary operator (-)
	    friend Matrix4 operator*(float scalar, const Matrix4& m);       // pre-multiplication
	    friend Vector3 operator*(const Vector3& vec, const Matrix4& m); // pre-multiplication
	    friend Vector4 operator*(const Vector4& vec, const Matrix4& m); // pre-multiplication
	private:
	    float       GetCofactor(float m0, float m1, float m2,
	                            float m3, float m4, float m5,
	                            float m6, float m7, float m8);
	private:
		float m[16];
	    float tm[16];                                       // transpose m
	};



	///////////////////////////////////////////////////////////////////////////
	// inline functions for Matrix2
	///////////////////////////////////////////////////////////////////////////
	inline Matrix2::Matrix2()
	{
	    // initially identity matrix
	    Identity();
	}



	inline Matrix2::Matrix2(const float src[4])
	{
	    Set(src);
	}



	inline Matrix2::Matrix2(float m0, float m1, float m2, float m3)
	{
	    Set(m0, m1, m2, m3);
	}



	inline void Matrix2::Set(const float src[4])
	{
	    m[0] = src[0];  m[1] = src[1];  m[2] = src[2];  m[3] = src[3];
	}



	inline void Matrix2::Set(float m0, float m1, float m2, float m3)
	{
	    m[0]= m0;  m[1] = m1;  m[2] = m2;  m[3]= m3;
	}



	inline void Matrix2::SetRow(int index, const float row[2])
	{
	    m[index] = row[0];  m[index + 2] = row[1];
	}



	inline void Matrix2::SetRow(int index, const Vector2& v)
	{
	    m[index] = v.x;  m[index + 2] = v.y;
	}



	inline void Matrix2::SetColumn(int index, const float col[2])
	{
	    m[index*2] = col[0];  m[index*2 + 1] = col[1];
	}



	inline void Matrix2::SetColumn(int index, const Vector2& v)
	{
	    m[index*2] = v.x;  m[index*2 + 1] = v.y;
	}



	inline const float* Matrix2::Get() const
	{
	    return m;
	}



	inline Matrix2& Matrix2::Identity()
	{
	    m[0] = m[3] = 1.0f;
	    m[1] = m[2] = 0.0f;
	    return *this;
	}



	inline Matrix2 Matrix2::operator+(const Matrix2& rhs) const
	{
	    return Matrix2(m[0]+rhs[0], m[1]+rhs[1], m[2]+rhs[2], m[3]+rhs[3]);
	}



	inline Matrix2 Matrix2::operator-(const Matrix2& rhs) const
	{
	    return Matrix2(m[0]-rhs[0], m[1]-rhs[1], m[2]-rhs[2], m[3]-rhs[3]);
	}



	inline Matrix2& Matrix2::operator+=(const Matrix2& rhs)
	{
	    m[0] += rhs[0];  m[1] += rhs[1];  m[2] += rhs[2];  m[3] += rhs[3];
	    return *this;
	}



	inline Matrix2& Matrix2::operator-=(const Matrix2& rhs)
	{
	    m[0] -= rhs[0];  m[1] -= rhs[1];  m[2] -= rhs[2];  m[3] -= rhs[3];
	    return *this;
	}



	inline Vector2 Matrix2::operator*(const Vector2& rhs) const
	{
	    return Vector2(m[0]*rhs.x + m[2]*rhs.y,  m[1]*rhs.x + m[3]*rhs.y);
	}



	inline Matrix2 Matrix2::operator*(const Matrix2& rhs) const
	{
	    return Matrix2(m[0]*rhs[0] + m[2]*rhs[1],  m[1]*rhs[0] + m[3]*rhs[1],
	                   m[0]*rhs[2] + m[2]*rhs[3],  m[1]*rhs[2] + m[3]*rhs[3]);
	}



	inline Matrix2& Matrix2::operator*=(const Matrix2& rhs)
	{
	    *this = *this * rhs;
	    return *this;
	}



	inline bool Matrix2::operator==(const Matrix2& rhs) const
	{
	    return (m[0] == rhs[0]) && (m[1] == rhs[1]) && (m[2] == rhs[2]) && (m[3] == rhs[3]);
	}



	inline bool Matrix2::operator!=(const Matrix2& rhs) const
	{
	    return (m[0] != rhs[0]) || (m[1] != rhs[1]) || (m[2] != rhs[2]) || (m[3] != rhs[3]);
	}



	inline float Matrix2::operator[](int index) const
	{
	    return m[index];
	}



	inline float& Matrix2::operator[](int index)
	{
	    return m[index];
	}



	inline Matrix2 operator-(const Matrix2& rhs)
	{
	    return Matrix2(-rhs[0], -rhs[1], -rhs[2], -rhs[3]);
	}



	inline Matrix2 operator*(float s, const Matrix2& rhs)
	{
	    return Matrix2(s*rhs[0], s*rhs[1], s*rhs[2], s*rhs[3]);
	}



	inline Vector2 operator*(const Vector2& v, const Matrix2& rhs)
	{
	    return Vector2(v.x*rhs[0] + v.y*rhs[1],  v.x*rhs[2] + v.y*rhs[3]);
	}
	// END OF MATRIX2 INLINE //////////////////////////////////////////////////////




	///////////////////////////////////////////////////////////////////////////
	// inline functions for Matrix3
	///////////////////////////////////////////////////////////////////////////
	inline Matrix3::Matrix3()
	{
	    // initially identity matrix
	    Identity();
	}



	inline Matrix3::Matrix3(const float src[9])
	{
	    Set(src);
	}



	inline Matrix3::Matrix3(float m0, float m1, float m2,
	                        float m3, float m4, float m5,
	                        float m6, float m7, float m8)
	{
	    Set(m0, m1, m2,  m3, m4, m5,  m6, m7, m8);
	}



	inline void Matrix3::Set(const float src[9])
	{
	    m[0] = src[0];  m[1] = src[1];  m[2] = src[2];
	    m[3] = src[3];  m[4] = src[4];  m[5] = src[5];
	    m[6] = src[6];  m[7] = src[7];  m[8] = src[8];
	}



	inline void Matrix3::Set(float m0, float m1, float m2,
	                         float m3, float m4, float m5,
	                         float m6, float m7, float m8)
	{
	    m[0] = m0;  m[1] = m1;  m[2] = m2;
	    m[3] = m3;  m[4] = m4;  m[5] = m5;
	    m[6] = m6;  m[7] = m7;  m[8] = m8;
	}



	inline void Matrix3::SetRow(int index, const float row[3])
	{
	    m[index] = row[0];  m[index + 3] = row[1];  m[index + 6] = row[2];
	}



	inline void Matrix3::SetRow(int index, const Vector3& v)
	{
	    m[index] = v.x;  m[index + 3] = v.y;  m[index + 6] = v.z;
	}



	inline void Matrix3::SetColumn(int index, const float col[3])
	{
	    m[index*3] = col[0];  m[index*3 + 1] = col[1];  m[index*3 + 2] = col[2];
	}



	inline void Matrix3::SetColumn(int index, const Vector3& v)
	{
	    m[index*3] = v.x;  m[index*3 + 1] = v.y;  m[index*3 + 2] = v.z;
	}



	inline const float* Matrix3::Get() const
	{
	    return m;
	}



	inline Matrix3& Matrix3::Identity()
	{
	    m[0] = m[4] = m[8] = 1.0f;
	    m[1] = m[2] = m[3] = m[5] = m[6] = m[7] = 0.0f;
	    return *this;
	}



	inline Matrix3 Matrix3::operator+(const Matrix3& rhs) const
	{
	    return Matrix3(m[0]+rhs[0], m[1]+rhs[1], m[2]+rhs[2],
	                   m[3]+rhs[3], m[4]+rhs[4], m[5]+rhs[5],
	                   m[6]+rhs[6], m[7]+rhs[7], m[8]+rhs[8]);
	}



	inline Matrix3 Matrix3::operator-(const Matrix3& rhs) const
	{
	    return Matrix3(m[0]-rhs[0], m[1]-rhs[1], m[2]-rhs[2],
	                   m[3]-rhs[3], m[4]-rhs[4], m[5]-rhs[5],
	                   m[6]-rhs[6], m[7]-rhs[7], m[8]-rhs[8]);
	}



	inline Matrix3& Matrix3::operator+=(const Matrix3& rhs)
	{
	    m[0] += rhs[0];  m[1] += rhs[1];  m[2] += rhs[2];
	    m[3] += rhs[3];  m[4] += rhs[4];  m[5] += rhs[5];
	    m[6] += rhs[6];  m[7] += rhs[7];  m[8] += rhs[8];
	    return *this;
	}



	inline Matrix3& Matrix3::operator-=(const Matrix3& rhs)
	{
	    m[0] -= rhs[0];  m[1] -= rhs[1];  m[2] -= rhs[2];
	    m[3] -= rhs[3];  m[4] -= rhs[4];  m[5] -= rhs[5];
	    m[6] -= rhs[6];  m[7] -= rhs[7];  m[8] -= rhs[8];
	    return *this;
	}



	inline Vector3 Matrix3::operator*(const Vector3& rhs) const
	{
	    return Vector3(m[0]*rhs.x + m[3]*rhs.y + m[6]*rhs.z,
	                   m[1]*rhs.x + m[4]*rhs.y + m[7]*rhs.z,
	                   m[2]*rhs.x + m[5]*rhs.y + m[8]*rhs.z);
	}



	inline Matrix3 Matrix3::operator*(const Matrix3& rhs) const
	{
	    return Matrix3(m[0]*rhs[0] + m[3]*rhs[1] + m[6]*rhs[2],  m[1]*rhs[0] + m[4]*rhs[1] + m[7]*rhs[2],  m[2]*rhs[0] + m[5]*rhs[1] + m[8]*rhs[2],
	                   m[0]*rhs[3] + m[3]*rhs[4] + m[6]*rhs[5],  m[1]*rhs[3] + m[4]*rhs[4] + m[7]*rhs[5],  m[2]*rhs[3] + m[5]*rhs[4] + m[8]*rhs[5],
	                   m[0]*rhs[6] + m[3]*rhs[7] + m[6]*rhs[8],  m[1]*rhs[6] + m[4]*rhs[7] + m[7]*rhs[8],  m[2]*rhs[6] + m[5]*rhs[7] + m[8]*rhs[8]);
	}



	inline Matrix3& Matrix3::operator*=(const Matrix3& rhs)
	{
	    *this = *this * rhs;
	    return *this;
	}



	inline bool Matrix3::operator==(const Matrix3& rhs) const
	{
	    return (m[0] == rhs[0]) && (m[1] == rhs[1]) && (m[2] == rhs[2]) &&
	           (m[3] == rhs[3]) && (m[4] == rhs[4]) && (m[5] == rhs[5]) &&
	           (m[6] == rhs[6]) && (m[7] == rhs[7]) && (m[8] == rhs[8]);
	}



	inline bool Matrix3::operator!=(const Matrix3& rhs) const
	{
	    return (m[0] != rhs[0]) || (m[1] != rhs[1]) || (m[2] != rhs[2]) ||
	           (m[3] != rhs[3]) || (m[4] != rhs[4]) || (m[5] != rhs[5]) ||
	           (m[6] != rhs[6]) || (m[7] != rhs[7]) || (m[8] != rhs[8]);
	}



	inline float Matrix3::operator[](int index) const
	{
	    return m[index];
	}



	inline float& Matrix3::operator[](int index)
	{
	    return m[index];
	}



	inline Matrix3 operator-(const Matrix3& rhs)
	{
	    return Matrix3(-rhs[0], -rhs[1], -rhs[2], -rhs[3], -rhs[4], -rhs[5], -rhs[6], -rhs[7], -rhs[8]);
	}



	inline Matrix3 operator*(float s, const Matrix3& rhs)
	{
	    return Matrix3(s*rhs[0], s*rhs[1], s*rhs[2], s*rhs[3], s*rhs[4], s*rhs[5], s*rhs[6], s*rhs[7], s*rhs[8]);
	}



	inline Vector3 operator*(const Vector3& v, const Matrix3& m)
	{
	    return Vector3(v.x*m[0] + v.y*m[1] + v.z*m[2],  v.x*m[3] + v.y*m[4] + v.z*m[5],  v.x*m[6] + v.y*m[7] + v.z*m[8]);
	}

	// END OF MATRIX3 INLINE //////////////////////////////////////////////////////




	///////////////////////////////////////////////////////////////////////////
	// inline functions for Matrix4
	///////////////////////////////////////////////////////////////////////////
	inline Matrix4::Matrix4()
	{
	    // initially identity matrix
	    Identity();
	}



	inline Matrix4::Matrix4(const float src[16])
	{
	    Set(src);
	}



	inline Matrix4::Matrix4(float m00, float m01, float m02, float m03,
	                        float m04, float m05, float m06, float m07,
	                        float m08, float m09, float m10, float m11,
	                        float m12, float m13, float m14, float m15)
	{
	    Set(m00, m01, m02, m03,  m04, m05, m06, m07,  m08, m09, m10, m11,  m12, m13, m14, m15);
	}



	inline void Matrix4::Set(const float src[16])
	{
	    m[0] = src[0];  m[1] = src[1];  m[2] = src[2];  m[3] = src[3];
	    m[4] = src[4];  m[5] = src[5];  m[6] = src[6];  m[7] = src[7];
	    m[8] = src[8];  m[9] = src[9];  m[10]= src[10]; m[11]= src[11];
	    m[12]= src[12]; m[13]= src[13]; m[14]= src[14]; m[15]= src[15];
	}



	inline void Matrix4::Set(float m00, float m01, float m02, float m03,
	                         float m04, float m05, float m06, float m07,
	                         float m08, float m09, float m10, float m11,
	                         float m12, float m13, float m14, float m15)
	{
	    m[0] = m00;  m[1] = m01;  m[2] = m02;  m[3] = m03;
	    m[4] = m04;  m[5] = m05;  m[6] = m06;  m[7] = m07;
	    m[8] = m08;  m[9] = m09;  m[10]= m10;  m[11]= m11;
	    m[12]= m12;  m[13]= m13;  m[14]= m14;  m[15]= m15;
	}



	inline void Matrix4::SetRow(int index, const float row[4])
	{
	    m[index] = row[0];  m[index + 4] = row[1];  m[index + 8] = row[2];  m[index + 12] = row[3];
	}



	inline void Matrix4::SetRow(int index, const Vector4& v)
	{
	    m[index] = v.x;  m[index + 4] = v.y;  m[index + 8] = v.z;  m[index + 12] = v.w;
	}



	inline void Matrix4::SetRow(int index, const Vector3& v)
	{
	    m[index] = v.x;  m[index + 4] = v.y;  m[index + 8] = v.z;
	}



	inline void Matrix4::SetColumn(int index, const float col[4])
	{
	    m[index*4] = col[0];  m[index*4 + 1] = col[1];  m[index*4 + 2] = col[2];  m[index*4 + 3] = col[3];
	}



	inline void Matrix4::SetColumn(int index, const Vector4& v)
	{
	    m[index*4] = v.x;  m[index*4 + 1] = v.y;  m[index*4 + 2] = v.z;  m[index*4 + 3] = v.w;
	}



	inline void Matrix4::SetColumn(int index, const Vector3& v)
	{
	    m[index*4] = v.x;  m[index*4 + 1] = v.y;  m[index*4 + 2] = v.z;
	}



	inline const float* Matrix4::Get() const
	{
	    return m;
	}



	inline const float* Matrix4::GetTranspose()
	{
	    tm[0] = m[0];   tm[1] = m[4];   tm[2] = m[8];   tm[3] = m[12];
	    tm[4] = m[1];   tm[5] = m[5];   tm[6] = m[9];   tm[7] = m[13];
	    tm[8] = m[2];   tm[9] = m[6];   tm[10]= m[10];  tm[11]= m[14];
	    tm[12]= m[3];   tm[13]= m[7];   tm[14]= m[11];  tm[15]= m[15];
	    return tm;
	}



	inline Matrix4& Matrix4::Identity()
	{
	    m[0] = m[5] = m[10] = m[15] = 1.0f;
	    m[1] = m[2] = m[3] = m[4] = m[6] = m[7] = m[8] = m[9] = m[11] = m[12] = m[13] = m[14] = 0.0f;
	    return *this;
	}



	inline Matrix4 Matrix4::operator+(const Matrix4& rhs) const
	{
	    return Matrix4(m[0]+rhs[0],   m[1]+rhs[1],   m[2]+rhs[2],   m[3]+rhs[3],
	                   m[4]+rhs[4],   m[5]+rhs[5],   m[6]+rhs[6],   m[7]+rhs[7],
	                   m[8]+rhs[8],   m[9]+rhs[9],   m[10]+rhs[10], m[11]+rhs[11],
	                   m[12]+rhs[12], m[13]+rhs[13], m[14]+rhs[14], m[15]+rhs[15]);
	}



	inline Matrix4 Matrix4::operator-(const Matrix4& rhs) const
	{
	    return Matrix4(m[0]-rhs[0],   m[1]-rhs[1],   m[2]-rhs[2],   m[3]-rhs[3],
	                   m[4]-rhs[4],   m[5]-rhs[5],   m[6]-rhs[6],   m[7]-rhs[7],
	                   m[8]-rhs[8],   m[9]-rhs[9],   m[10]-rhs[10], m[11]-rhs[11],
	                   m[12]-rhs[12], m[13]-rhs[13], m[14]-rhs[14], m[15]-rhs[15]);
	}



	inline Matrix4& Matrix4::operator+=(const Matrix4& rhs)
	{
	    m[0] += rhs[0];   m[1] += rhs[1];   m[2] += rhs[2];   m[3] += rhs[3];
	    m[4] += rhs[4];   m[5] += rhs[5];   m[6] += rhs[6];   m[7] += rhs[7];
	    m[8] += rhs[8];   m[9] += rhs[9];   m[10]+= rhs[10];  m[11]+= rhs[11];
	    m[12]+= rhs[12];  m[13]+= rhs[13];  m[14]+= rhs[14];  m[15]+= rhs[15];
	    return *this;
	}



	inline Matrix4& Matrix4::operator-=(const Matrix4& rhs)
	{
	    m[0] -= rhs[0];   m[1] -= rhs[1];   m[2] -= rhs[2];   m[3] -= rhs[3];
	    m[4] -= rhs[4];   m[5] -= rhs[5];   m[6] -= rhs[6];   m[7] -= rhs[7];
	    m[8] -= rhs[8];   m[9] -= rhs[9];   m[10]-= rhs[10];  m[11]-= rhs[11];
	    m[12]-= rhs[12];  m[13]-= rhs[13];  m[14]-= rhs[14];  m[15]-= rhs[15];
	    return *this;
	}



	inline Vector4 Matrix4::operator*(const Vector4& rhs) const
	{
	    return Vector4(m[0]*rhs.x + m[4]*rhs.y + m[8]*rhs.z  + m[12]*rhs.w,
	                   m[1]*rhs.x + m[5]*rhs.y + m[9]*rhs.z  + m[13]*rhs.w,
	                   m[2]*rhs.x + m[6]*rhs.y + m[10]*rhs.z + m[14]*rhs.w,
	                   m[3]*rhs.x + m[7]*rhs.y + m[11]*rhs.z + m[15]*rhs.w);
	}



	inline Vector3 Matrix4::operator*(const Vector3& rhs) const
	{
	    return Vector3(m[0]*rhs.x + m[4]*rhs.y + m[8]*rhs.z,
	                   m[1]*rhs.x + m[5]*rhs.y + m[9]*rhs.z,
	                   m[2]*rhs.x + m[6]*rhs.y + m[10]*rhs.z);
	}



	inline Matrix4 Matrix4::operator*(const Matrix4& n) const
	{
	    return Matrix4(m[0]*n[0]  + m[4]*n[1]  + m[8]*n[2]  + m[12]*n[3],   m[1]*n[0]  + m[5]*n[1]  + m[9]*n[2]  + m[13]*n[3],   m[2]*n[0]  + m[6]*n[1]  + m[10]*n[2]  + m[14]*n[3],   m[3]*n[0]  + m[7]*n[1]  + m[11]*n[2]  + m[15]*n[3],
	                   m[0]*n[4]  + m[4]*n[5]  + m[8]*n[6]  + m[12]*n[7],   m[1]*n[4]  + m[5]*n[5]  + m[9]*n[6]  + m[13]*n[7],   m[2]*n[4]  + m[6]*n[5]  + m[10]*n[6]  + m[14]*n[7],   m[3]*n[4]  + m[7]*n[5]  + m[11]*n[6]  + m[15]*n[7],
	                   m[0]*n[8]  + m[4]*n[9]  + m[8]*n[10] + m[12]*n[11],  m[1]*n[8]  + m[5]*n[9]  + m[9]*n[10] + m[13]*n[11],  m[2]*n[8]  + m[6]*n[9]  + m[10]*n[10] + m[14]*n[11],  m[3]*n[8]  + m[7]*n[9]  + m[11]*n[10] + m[15]*n[11],
	                   m[0]*n[12] + m[4]*n[13] + m[8]*n[14] + m[12]*n[15],  m[1]*n[12] + m[5]*n[13] + m[9]*n[14] + m[13]*n[15],  m[2]*n[12] + m[6]*n[13] + m[10]*n[14] + m[14]*n[15],  m[3]*n[12] + m[7]*n[13] + m[11]*n[14] + m[15]*n[15]);
	}



	inline Matrix4& Matrix4::operator*=(const Matrix4& rhs)
	{
	    *this = *this * rhs;
	    return *this;
	}



	inline bool Matrix4::operator==(const Matrix4& n) const
	{
	    return (m[0] == n[0])  && (m[1] == n[1])  && (m[2] == n[2])  && (m[3] == n[3])  &&
	           (m[4] == n[4])  && (m[5] == n[5])  && (m[6] == n[6])  && (m[7] == n[7])  &&
	           (m[8] == n[8])  && (m[9] == n[9])  && (m[10]== n[10]) && (m[11]== n[11]) &&
	           (m[12]== n[12]) && (m[13]== n[13]) && (m[14]== n[14]) && (m[15]== n[15]);
	}



	inline bool Matrix4::operator!=(const Matrix4& n) const
	{
	    return (m[0] != n[0])  || (m[1] != n[1])  || (m[2] != n[2])  || (m[3] != n[3])  ||
	           (m[4] != n[4])  || (m[5] != n[5])  || (m[6] != n[6])  || (m[7] != n[7])  ||
	           (m[8] != n[8])  || (m[9] != n[9])  || (m[10]!= n[10]) || (m[11]!= n[11]) ||
	           (m[12]!= n[12]) || (m[13]!= n[13]) || (m[14]!= n[14]) || (m[15]!= n[15]);
	}



	inline float Matrix4::operator[](int index) const
	{
	    return m[index];
	}



	inline float& Matrix4::operator[](int index)
	{
	    return m[index];
	}



	inline Matrix4 operator-(const Matrix4& rhs)
	{
	    return Matrix4(-rhs[0], -rhs[1], -rhs[2], -rhs[3], -rhs[4], -rhs[5], -rhs[6], -rhs[7], -rhs[8], -rhs[9], -rhs[10], -rhs[11], -rhs[12], -rhs[13], -rhs[14], -rhs[15]);
	}



	inline Matrix4 operator*(float s, const Matrix4& rhs)
	{
	    return Matrix4(s*rhs[0], s*rhs[1], s*rhs[2], s*rhs[3], s*rhs[4], s*rhs[5], s*rhs[6], s*rhs[7], s*rhs[8], s*rhs[9], s*rhs[10], s*rhs[11], s*rhs[12], s*rhs[13], s*rhs[14], s*rhs[15]);
	}



	inline Vector4 operator*(const Vector4& v, const Matrix4& m)
	{
	    return Vector4(v.x*m[0] + v.y*m[1] + v.z*m[2] + v.w*m[3],  v.x*m[4] + v.y*m[5] + v.z*m[6] + v.w*m[7],  v.x*m[8] + v.y*m[9] + v.z*m[10] + v.w*m[11], v.x*m[12] + v.y*m[13] + v.z*m[14] + v.w*m[15]);
	}



	inline Vector3 operator*(const Vector3& v, const Matrix4& m)
	{
	    return Vector3(v.x*m[0] + v.y*m[1] + v.z*m[2],  v.x*m[4] + v.y*m[5] + v.z*m[6],  v.x*m[8] + v.y*m[9] + v.z*m[10]);
	}
}
#endif
