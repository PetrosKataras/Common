#include "../common/offset.glsl"

uniform sampler2D uSamplerColor;

uniform vec3 uFogColor;
uniform float uDensity;

in VertexData {
	vec4 position;
	vec3 normal;
	vec4 color;
	vec2 texCoord;
	vec4 proj;
} vVertexIn;

out vec4 fragColor;

void main( void )
{
	vec2 uv = calcTexCoordFromUv( vVertexIn.texCoord.st );
	float fogAmount = ( gl_FragCoord.z / gl_FragCoord.w ) * uDensity;
	fogAmount = clamp( fogAmount, 0.0, 1.0 );
	vec4 color = texture( uSamplerColor, uv );
	fragColor.rgb = mix( color.rgb, uFogColor, fogAmount );
	fragColor.a = 1.0;
}
