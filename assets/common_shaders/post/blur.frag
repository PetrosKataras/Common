#include "../common/offset.glsl"

uniform sampler2D uTexture;
uniform vec2 uResolution;
uniform vec2 uDirection;

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
    vec2 nCoord = ( vVertexIn.texCoord.st );
	vec4 color = vec4( 0.0 );
	vec2 off1 = vec2(1.3333333333333333) * uDirection;
    color += texture( uTexture, nCoord ) * 0.29411764705882354;
    color += texture( uTexture, nCoord + ( off1 / uResolution ) ) * 0.35294117647058826;
    color += texture( uTexture, nCoord - ( off1 / uResolution ) ) * 0.35294117647058826;

    fragColor = color;
}
