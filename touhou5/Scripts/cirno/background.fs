#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 worldPos;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

out vec4 finalColor;

uniform vec2	u_texelSize;
uniform float	u_time;
uniform vec3	u_fogOrigin;
uniform vec4	u_fogColor;
uniform float	u_fogNear;
uniform float	u_fogFar;
uniform int		u_fogEnabled;

void main()
{
	float t = u_time / 10.;

	vec4 texelColor = texture(texture0, fragTexCoord + u_texelSize * t);
	texelColor = mix(texelColor, texture(texture0, fragTexCoord + u_texelSize * vec2(-1., 1.) * t), .5);

	finalColor = texelColor * colDiffuse;

	if (u_fogEnabled == 1) {
		float dist = length(worldPos - u_fogOrigin);
		float fraction = clamp((dist - u_fogNear) / (u_fogFar - u_fogNear), 0., 1.);

		finalColor = mix(finalColor, u_fogColor, fraction);
	}
}
