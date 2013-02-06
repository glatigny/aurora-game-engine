#ifndef DEFAULT_CONTROLLER_CONFIG_H
#define DEFAULT_CONTROLLER_CONFIG_H

#include "input_engine.h"
#include "vfs.h"
#include <map>
#include <string>

class DefaultControllerConfig : public ControllerConfig {
public:
	typedef std::map<std::string, ControllerState::Axis> AxisMapping;
	typedef std::map<std::string, ControllerState::Button> ButtonMapping;
protected:
	std::map<string, AxisMapping> axes;
	std::map<string, ButtonMapping> buttons;

	VFS::XMLSource* configFile;

public:
	DefaultControllerConfig(VFS::File* configuration);
  DefaultControllerConfig(VFS::XMLSource* configuration);
  virtual ~DefaultControllerConfig();

	/** Maps devices by sending the configuration to the specified input engine.
   * This function calls binding methods on the engine object.
   * @param engine the engine to configure.
   */
	void mapDevices(InputEngine* engine);

	/** Store an axis mapping.
	 * @param deviceName the name of the device which has the axis.
	 * @param axisName the name of the axis on the device.
	 * @param logicalAxis the logical axis to map.
	 */
	void storeAxisMapping(std::string deviceName, std::string axisName, ControllerState::Axis logicalAxis);

	/** Store a button mapping.
	 * @param deviceName the name of the device which has the axis.
	 * @param buttonName the name of the button on the device.
	 * @param logicalButton the logical button to map.
	 */
	void storeButtonMapping(std::string deviceName, std::string buttonName, ControllerState::Button logicalButton);

	/** Save the configuration file.
	 * This function saves the current configuration to the configuration file passed to the constructor.
	 */
	void saveConfig();

};

#endif /* DEFAULT_CONTROLLER_CONFIG */
