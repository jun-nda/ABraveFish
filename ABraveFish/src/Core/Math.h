#ifndef MATHS_H
#define MATHS_H

namespace ABraveFish {
struct Vec2 {
    Vec2() {}
    Vec2(double x, double y)
        : x(x)
        , y(y) {}
    double x = 0.f, y = 0.f;
    inline Vec2 operator+( ) {

    }

};
struct Vec3 {
    Vec3() {}
    Vec3(double x, double y, double z)
        : x(x)
        , y(y)
        , z(z) {}
    double x = 0.f, y = 0.f, z = 0.f;
};
struct Vec4 {
    Vec4() {}
    Vec4(double x, double y, double z, double w)
        : x(x)
        , y(y)
        , z(z)
        , w(w) {}
    double x = 0.f, y = 0.f, z = 0.f, w = 0.f;
};
struct Quat {
    Quat() {}
    Quat(double x, double y, double z, double w)
        : x(x)
        , y(y)
        , z(z)
        , w(w) {}
    double x = 0.f, y = 0.f, z = 0.f, w = 0.f;
} ;
typedef struct {
    double m[3][3];
} Mat3;
typedef struct {
    double m[4][4];
} Mat4;

/* double related functions */
double         float_min(double a, double b);
double         float_max(double a, double b);
double         float_lerp(double a, double b, double t);
double         float_clamp(double f, double min, double max);
double         float_saturate(double f);
double         float_from_uchar(unsigned char value);
unsigned char float_to_uchar(double value);
double         float_srgb2linear(double value);
double         float_linear2srgb(double value);
double         float_aces(double value);
void          float_print(const char* name, double f);

/* vec2 related functions */
Vec2  vec2_new(double x, double y);
Vec2  vec2_min(Vec2 a, Vec2 b);
Vec2  vec2_max(Vec2 a, Vec2 b);
Vec2  vec2_add(Vec2 a, Vec2 b);
Vec2  vec2_sub(Vec2 a, Vec2 b);
Vec2  vec2_mul(Vec2 v, double factor);
Vec2  vec2_div(Vec2 v, double divisor);
double vec2_length(Vec2 v);
double vec2_edge(Vec2 start, Vec2 end, Vec2 v);
void  vec2_print(const char* name, Vec2 v);

/* vec3 related functions */
Vec3  vec3_new(double x, double y, double z);
Vec3  vec3_from_vec4(Vec4 v);
Vec3  vec3_min(Vec3 a, Vec3 b);
Vec3  vec3_max(Vec3 a, Vec3 b);
Vec3  vec3_add(Vec3 a, Vec3 b);
Vec3  vec3_sub(Vec3 a, Vec3 b);
Vec3  vec3_mul(Vec3 v, double factor);
Vec3  vec3_div(Vec3 v, double divisor);
Vec3  vec3_negate(Vec3 v);
double vec3_length(Vec3 v);
Vec3  vec3_normalize(Vec3 v);
double vec3_dot(Vec3 a, Vec3 b);
Vec3  vec3_cross(Vec3 a, Vec3 b);
Vec3  vec3_lerp(Vec3 a, Vec3 b, double t);
Vec3  vec3_saturate(Vec3 v);
Vec3  vec3_modulate(Vec3 a, Vec3 b);
void  vec3_print(const char* name, Vec3 v);

/* vec4 related functions */
Vec4 vec4_new(double x, double y, double z, double w);
Vec4 vec4_from_vec3(Vec3 v, double w);
Vec4 vec4_add(Vec4 a, Vec4 b);
Vec4 vec4_sub(Vec4 a, Vec4 b);
Vec4 vec4_mul(Vec4 v, double factor);
Vec4 vec4_div(Vec4 v, double divisor);
Vec4 vec4_lerp(Vec4 a, Vec4 b, double t);
Vec4 vec4_saturate(Vec4 v);
Vec4 vec4_modulate(Vec4 a, Vec4 b);
void vec4_print(const char* name, Vec4 v);

/* quat related functions */
Quat  quat_new(double x, double y, double z, double w);
double quat_dot(Quat a, Quat b);
double quat_length(Quat q);
Quat  quat_normalize(Quat q);
Quat  quat_slerp(Quat a, Quat b, double t);
void  quat_print(const char* name, Quat q);

/* mat3 related functions */
Mat3 mat3_identity(void);
Mat3 mat3_from_cols(Vec3 c0, Vec3 c1, Vec3 c2);
Mat3 mat3_from_mat4(Mat4 m);
Mat3 mat3_combine(Mat3 m[4], Vec4 weights);
Vec3 mat3_mul_vec3(Mat3 m, Vec3 v);
Mat3 mat3_mul_mat3(Mat3 a, Mat3 b);
Mat3 mat3_inverse(Mat3 m);
Mat3 Mat3ranspose(Mat3 m);
Mat3 mat3_inverse_transpose(Mat3 m);
void mat3_print(const char* name, Mat3 m);

/* mat4 related functions */
Mat4 mat4_identity(void);
Mat4 mat4_from_quat(Quat q);
Mat4 mat4_from_trs(Vec3 t, Quat r, Vec3 s);
Mat4 mat4_combine(Mat4 m[4], Vec4 weights);
Vec4 mat4_mul_vec4(Mat4 m, Vec4 v);
Mat4 mat4_mul_mat4(Mat4 a, Mat4 b);
Mat4 mat4_inverse(Mat4 m);
Mat4 Mat4ranspose(Mat4 m);
Mat4 mat4_inverse_transpose(Mat4 m);
void mat4_print(const char* name, Mat4 m);

/* transformation matrices */
Mat4 Mat4ranslate(double tx, double ty, double tz);
Mat4 mat4_scale(double sx, double sy, double sz);
Mat4 mat4_rotate(double angle, double vx, double vy, double vz);
Mat4 mat4_rotate_x(double angle);
Mat4 mat4_rotate_y(double angle);
Mat4 mat4_rotate_z(double angle);
Mat4 mat4_lookat(Vec3 eye, Vec3 target, Vec3 up);
Mat4 mat4_ortho(double left, double right, double bottom, double top, double near, double far);
Mat4 mat4_frustum(double left, double right, double bottom, double top, double near, double far);
Mat4 mat4_orthographic(double right, double top, double near, double far);
Mat4 mat4_perspective(double fovy, double aspect, double near, double far);
} // namespace ABraveFish

#endif