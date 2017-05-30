#version 150
// ^ Change this to version 130 if you have compatibility issues

// This is a vertex shader. While it is called a "shader" due to outdated conventions, this file
// is used to apply matrix transformations to the arrays of vertex data passed to it.
// Since this code is run on your GPU, each vertex is transformed simultaneously.
// If it were run on your CPU, each vertex would have to be processed in a FOR loop, one at a time.
// This simultaneous transformation allows your program to run much faster, especially when rendering
// geometry with millions of vertices.

uniform mat4 u_Model;       // The matrix that defines the transformation of the
                            // object we're rendering. In this assignment,
                            // this will be the result of traversing your scene graph.

uniform mat4 u_ModelInvTr;  // The inverse transpose of the model matrix.
                            // This allows us to transform the object's normals properly
                            // if the object has been non-uniformly scaled.

uniform mat4 u_ViewProj;    // The matrix that defines the camera's transformation.
                            // We've written a static matrix for you to use for HW2,
                            // but in HW3 you'll have to generate one yourself

in vec3 vs_Pos;  // ---------->The array of vertex positions passed to the shader

in vec3 vs_Nor;  // ---------->The array of vertex normals passed to the shader

in vec3 vs_Col;  // ---------->The array of vertex colors passed to the shader.

out vec3 fs_Nor;  // --------->The array of normals that has been transformed by u_ModelInvTr. This is implicitly passed to the fragment shader.
out vec3 fs_LightVec;  // ---->The direction in which our virtual light lies, relative to each vertex. This is implicitly passed to the fragment shader.
out vec3 fs_Col;  // --------->The color of each vertex. This is implicitly passed to the fragment shader.

const vec3 lightVec = vec3(1, 1, 1);  // The position of our virtual light, which is used to compute the shading of
                                        // the geometry in the fragment shader.

void main()
{
    fs_Col = vs_Col;  //                          Pass the vertex color positions to the fragment shader
    fs_Nor = vec3(u_ModelInvTr * vec4(vs_Nor, 0));  //           Transform the geometry's normals

    vec4 modelposition = u_Model * vec4(vs_Pos, 1);  //    Temporarily store the transformed vertex positions for use below

    fs_LightVec = normalize(lightVec);//(lightPos - modelposition).xyz;  //   Compute the direction in which the light source lies

    gl_Position = u_ViewProj * modelposition;  // gl_Position is a built-in variable of OpenGL which is used to render the final positions
                                             // of the geometry's vertices
}
