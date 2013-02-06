#include "graphical_engine.h"
#include "resource_manager.h"
#include "vfs.h"

/*------------------------------------------------------------------------------*/

GraphicalEngine::~GraphicalEngine()
{
	// Release textures
	Texture *tmp;
	for(std::map<VFS::ImageSource*, struct_Texture*, strCmp>::iterator t = m_textures.begin(); t != m_textures.end(); t++)
	{
		tmp = t->second->l_texture;
		if( tmp != NULL )
			delete tmp;
		delete t->first;
		delete t->second;
	}

	m_textures.clear();
}

/*------------------------------------------------------------------------------*/

// QC:W
bool GraphicalEngine::myFreeTexture(Texture* t) {
	// Responsible for most memory leaks !!!
	return false;
}

/*------------------------------------------------------------------------------*/

Texture* GraphicalEngine::loadTexture(VFS::ImageSource* p_file)
{
	xassert(p_file, "loadTexture : Trying to load a NULL ImageSource.");

	// check if in database and get it if exist
	std::map<VFS::ImageSource*, struct_Texture*, strCmp>::iterator tmp = m_textures.find(p_file);
	
	if( tmp != m_textures.end() )
	{
		// return it if exist
	
		// Get element from iterator
		struct_Texture* struct_tmp = (*tmp).second;
		
		struct_tmp->usageCount++;
		return struct_tmp->l_texture;
	}

	// else create it
	struct_Texture* struct_tmp = new struct_Texture;
	struct_tmp->usageCount = 1;
	struct_tmp->l_texture = myLoadTexture(p_file);
	m_textures[p_file->copy()] = struct_tmp;
	return struct_tmp->l_texture;
}

/*------------------------------------------------------------------------------*/

bool GraphicalEngine::hasTexture(Texture* p_texture)
{
	for( std::map<VFS::ImageSource*, struct_Texture*, strCmp>::iterator iter = m_textures.begin(); 
		iter != m_textures.end(); 
		iter++)
	{
		if( (*iter).second->l_texture == p_texture )
		{
			return true;
		}
	}

	return false;
}

/*------------------------------------------------------------------------------*/

bool GraphicalEngine::freeTexture(Texture* p_texture)
{
	if(!hasTexture(p_texture)) // BIG FAT UGLY FIXME : textures may be freed multiple times.
		return false;

	assert(hasTexture(p_texture));

	// check if in database and get its index
	VFS::ImageSource* l_name = NULL;
	struct_Texture* tmp = NULL;

	if( l_name != NULL )
	{
		tmp = m_textures[l_name];
	}
	else
	{
		for( std::map<VFS::ImageSource*, struct_Texture*, strCmp>::iterator iter = m_textures.begin(); 
			(iter != m_textures.end()) && (tmp == NULL); 
			iter++)
		{
			if( iter->second->l_texture == p_texture )
			{
				tmp = iter->second;
				l_name = iter->first;
			}
		}
	}
	
	// decrement counter
	tmp->usageCount--;

	// ask for delete it if counter reach 0
	if( tmp->usageCount == 0 )
	{
		bool ret = myFreeTexture(tmp->l_texture);
		if( ret ) {
			m_textures.erase(l_name);
			delete l_name;
			delete tmp;
		}
		return ret;
	}

	return false;
}

/*------------------------------------------------------------------------------*/
