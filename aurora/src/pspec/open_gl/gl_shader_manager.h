#ifndef GL_SHADER_MANAGER_H
#define GL_SHADER_MANAGER_H

#include <map>
#include <string>
#include "vfs.h"
#include "open_gl.h"

class ShaderManager {
private:
	struct Shader {
		GLuint shader;
		OGLImageProcess process;
		uint32_t slots;
		Shader(GLuint sh, OGLImageProcess pr, uint32_t sl) : shader(sh), process(pr), slots(sl) {}
	};
	typedef std::map<std::string, Shader*> ShaderMap;
	ShaderMap shaders;

	typedef std::map<std::string, GLuint> VertexShaderMap;
	VertexShaderMap vertexShaders;
	VFS::Directory* baseDir;

public:
	ShaderManager(VFS::File* effectsDirectory) {
		baseDir = NULL;
		if(effectsDirectory) {
			VFS::Directory* dir = dynamic_cast<VFS::Directory*>(effectsDirectory);
			xerror(dir, "Processing file %s is not a directory.", effectsDirectory->name());
			baseDir = dir->copy();
		}
	}

	~ShaderManager() {
		if(baseDir) {
			delete baseDir;
		}
		for(ShaderMap::iterator it = shaders.begin(); it != shaders.end(); it++) {
			glDeleteProgram(it->second->process.program);
			glDeleteShader(it->second->shader);
			delete it->second;
		}
		for(VertexShaderMap::iterator it = vertexShaders.begin(); it != vertexShaders.end(); it++) {
			glDeleteShader(it->second);
		}
	}

	void loadShaders() {
		if(!baseDir)
			return;

		xassert(glGetError() == GL_NO_ERROR, "The OpenGL stack was in error. Aborting.");

		std::list<std::string> files = baseDir->listFiles(".glsl", true);
		for(std::list<std::string>::iterator it = files.begin(); it != files.end(); it++) {
			VFS::File* shaderFile = VFS::openFile(it->c_str(), baseDir);

			// Read shader file
			GLuint fragmentShader;
			GLuint vertexShader = 0;
			GLchar* shaderCode = (GLchar*) (shaderFile->readToString());
#ifndef NDEBUG
			xwarn(strstr(shaderCode, "gl_FragColor"), "The file %s is not a fragment shader.", shaderFile->name());
			if(!strstr(shaderCode, "gl_FragColor")) {
				AOEFREE(shaderCode);
				delete shaderFile;
				continue;
			}
#endif
			char* indicator;

			// Find slots in source
			uint32_t slots;
			indicator = strstr(shaderCode, "// slots = ");
			if(indicator) {
				slots = atoi(indicator + 11);
			} else {
				// Indicator not found, suppose the shader uses all slots and waste resources
				slots = (1<<TEXTURE_SLOTS) - 1;
			}

			// Find vertex shader in source
			indicator = strstr(shaderCode, "// vertex = ");
			if(indicator) {
				char filename[256];
				char* endOfFilename = strchr(indicator + 12, '\n');
				char* endOfDOSFilename = strchr(indicator + 12, '\r');
				if(endOfDOSFilename && endOfDOSFilename < endOfFilename)
					endOfFilename = endOfDOSFilename;
				xerror(endOfFilename, "Error while parsing vertex shader name.");
				memcpy(filename, indicator + 12, endOfFilename - indicator - 12);
				memcpy(filename + (endOfFilename - indicator - 12), ".vglsl", 7);
				vertexShader = loadVertexShader(filename, shaderFile);
#ifndef NDEBUG
				if(!vertexShader) {
					xwarn(false, "Shader %s could not load its vertex shader %s.", it->c_str(), filename);
					AOEFREE(shaderCode);
					delete shaderFile;
					continue;
				}
#endif
			}

			OGLImageProcess pr;

			// Setup mipmap flag
			pr.use_mipmaps = strstr(shaderCode, "// use_mipmaps") != NULL;

			// Load clear color
			indicator = shaderCode;
			do {
				indicator = strstr(indicator, " clear_color[");
				if(indicator) {
					indicator += 13;
					int slot = atoi(indicator);
					indicator = strstr(indicator, "] = ");
					xerror(indicator, "Malformed shader clear_color : no closing bracket.");
					indicator += 4;
					xerror(indicator, "Malformed shader clear_color : no value.");
					int r, g, b, a;
#ifndef NERROR
					int count = 
#endif
					stringToIntList(indicator, &r, &g, &b, &a);
					xerror(count == 4, "Malformed shader clear_color : not enough values (only %d found, need 4).", count);
					pr.clearColors[slot][0] = (GLclampf)r / 254.0f;
					pr.clearColors[slot][1] = (GLclampf)g / 254.0f;
					pr.clearColors[slot][2] = (GLclampf)b / 254.0f;
					pr.clearColors[slot][3] = (GLclampf)a / 254.0f;
				}
			} while(indicator);

			// Load shader
			const GLchar* constShaderCode = (const GLchar*) shaderCode;
			fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragmentShader, 1, &constShaderCode, NULL);
			glCompileShader(fragmentShader);
#ifndef NDEBUG
			{
				int result;
				glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
				xerror(glGetError() == GL_NO_ERROR, "Could not call fragment shader compiler.");
				if(result == GL_FALSE) {
					int infoLogLength = 0;
					glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &infoLogLength);
					syslog("GLSL compiler output :");
					if(infoLogLength > 0) {
						char* infoLog = (char *)AOEMALLOC(infoLogLength);
						glGetShaderInfoLog(fragmentShader, infoLogLength, NULL, infoLog);
						syslog("%s", infoLog);
						AOEFREE(infoLog);
					}
					xwarn(false, "GLSL fragment shader compiler error while compiling %s.", it->c_str());
					delete shaderFile;
					AOEFREE(shaderCode);
					glDeleteShader(fragmentShader);
					continue;
				}
			}
#endif
			delete shaderFile;

			// Link shader into a program
			GLuint program = glCreateProgram();
			if(vertexShader) {
				glAttachShader(program, vertexShader);
			}
			glAttachShader(program, fragmentShader);
			glLinkProgram(program);
			xerror(glGetError() == GL_NO_ERROR, "Shader %s generated an error at link time.", it->c_str());
#ifndef NDEBUG
			{
				int result;
				glGetProgramiv(program, GL_LINK_STATUS, &result);
				xerror(glGetError() == GL_NO_ERROR, "Could not call fragment shader linker.");
				if(result == GL_FALSE) {
					int infoLogLength = 0;
					glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
					syslog("GLSL linker output :");
					if(infoLogLength > 0) {
						char* infoLog = (char *)AOEMALLOC(infoLogLength);
						glGetProgramInfoLog(program, infoLogLength, NULL, infoLog);
						syslog("%s", infoLog);
						AOEFREE(infoLog);
					}
					xwarn(false, "GLSL linker compiler error while compiling %s.", it->c_str());
					AOEFREE(shaderCode);
					glDeleteShader(fragmentShader);
					glDeleteProgram(program);
					continue;
				}
			}
#endif

			// Setup multitexturing
			pr.program = program;
			pr.slots = slots;
			for(int i=0; i<TEXTURE_SLOTS; i++) {
				if(slots & 1<<i) {
					char name[6];
					snprintf(name, 5, "tex%d", i);
					pr.textureLocations[i] = glGetUniformLocation(program, name);
				}
			}

			// Setup size uniforms
			pr.widthLocation = strstr(shaderCode, "width")?glGetUniformLocation(program, "width"):-1;
			pr.heightLocation = strstr(shaderCode, "height")?glGetUniformLocation(program, "height"):-1;

			shaders[it->substr(0, it->rfind('.'))] = new Shader(fragmentShader, pr, slots);

			AOEFREE(shaderCode);
		}
	}

	GLuint loadVertexShader(const char* filename, VFS::File* fragmentShaderFile) {
		VertexShaderMap::iterator it = vertexShaders.find(filename);
		if(it != vertexShaders.end()) {
			return it->second;
		}
		// Read vertex shader
		VFS::File* vertexShaderFile = VFS::openFile(filename, fragmentShaderFile);
		GLchar* vertexShaderCode = (GLchar*) (vertexShaderFile->readToString());
#ifndef NDEBUG
		xwarn(strstr(vertexShaderCode, "gl_Position"), "The file %s is not a vertex shader.", vertexShaderFile->name());
		if(!strstr(vertexShaderCode, "gl_Position")) {
			AOEFREE(vertexShaderCode);
			delete vertexShaderFile;
			return 0;
		}
#endif

		const GLchar* constShaderCode = (const GLchar*)vertexShaderCode;
		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &constShaderCode, NULL);
		glCompileShader(vertexShader);
#ifndef NDEBUG
		{
			int result;
			glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
			xerror(glGetError() == GL_NO_ERROR, "Could not call vertex shader compiler.");
			if(result == GL_FALSE) {
				int infoLogLength = 0;
				glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &infoLogLength);
				syslog("GLSL compiler output :");
				if(infoLogLength > 0) {
					char* infoLog = (char *)AOEMALLOC(infoLogLength);
					glGetShaderInfoLog(vertexShader, infoLogLength, NULL, infoLog);
					syslog("%s", infoLog);
					AOEFREE(infoLog);
				}
				xwarn(false, "Vertex shader %s generated an error at compilation.", filename);
				delete vertexShaderFile;
				AOEFREE(vertexShaderCode);
				glDeleteShader(vertexShader);
				return 0;
			}
		}
#endif

		vertexShaders[filename] = vertexShader;
		return vertexShader;
	}

	bool getProcess(const char* name, void* &process, uint32_t &slots) {
		ShaderMap::iterator it = shaders.find(std::string(name));
		if(it == shaders.end()) {
			xwarn(false, "Shader manager could not find shader %s.", name);
			return false;
		}

		process = &(it->second->process);
		slots = it->second->slots;
		return true;
	}
};

#endif /* GL_SHADER_MANAGER_H */
