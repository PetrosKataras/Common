#version 150

uniform mat4	ciModelViewProjection;
uniform mat4	ciModelView;
uniform mat3	ciNormalMatrix;

uniform mat4	ciProjectionMatrix;
uniform mat4	ciViewMatrix;

#if defined( INSTANCED )
in mat4		vInstanceMatrix; // per instance
#endif

in vec4		ciPosition;
in vec3		ciNormal;
in vec4		ciColor;
in vec2		ciTexCoord0;

out VertexData {
	vec4 position;
	vec3 normal;
	vec4 color;
	vec2 texCoord;
	vec4 proj;
} vVertexOut;

void main()
{
#if defined( INSTANCED )
	vVertexOut.position = ciModelView * vInstanceMatrix * ciPosition;
#else
	vVertexOut.position = ciModelView * ciPosition;
#endif
	vVertexOut.normal = ( ciNormalMatrix * ciNormal );
	vVertexOut.color = ciColor;
	vVertexOut.texCoord = ciTexCoord0;

	gl_Position = ciProjectionMatrix * vVertexOut.position;
	vVertexOut.proj = gl_Position;
}
