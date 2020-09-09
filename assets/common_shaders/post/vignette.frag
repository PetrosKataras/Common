#include "../common/offset.glsl"

uniform float FallOff = .85;
uniform sampler2D uTexture;

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
    vec2 nCoord = calcTexCoordFromUv( vVertexIn.texCoord.st );
    vec4 color = texture( uTexture, nCoord );
    vec2 centeredCoord = ( nCoord - 0.5 ) * ( uDims.x / uDims.y ) * 2.0;
    float distance = sqrt( dot( centeredCoord,centeredCoord ) ) * FallOff;
	float rf2_1 = distance * distance + 1.0;
	float e = 1.0 / ( rf2_1 * rf2_1 );
    
    fragColor = vec4( color.rgb * e, 1.0f );
}

