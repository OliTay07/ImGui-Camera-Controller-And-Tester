# template-imguiWinAppDX11-with-rest-and-ws-client

## about

an imgui based DX11 app template with rest and websocket client.

### build

to build run the python script

```bash
python build.py
```

#### alternative build - individual build steps

get all packages from conan for both debug and release

```bash
conan install . --build=missing -s build_type=Debug
conan install . --build=missing -s build_type=Release
```

now build the cmake project

```bash
cmake --preset conan-default -DDEV_MACHINE=ON
```

## author

Stefan Kazassoglou


-----------------------------------------------------------------------------------

ImGui camera controller & test exporter 
Created by Olivia Taylor at Jones AV Ltd. 
-----------------------------------------

This is a simple camera controller and tester for the Jones AV camera system. Once the camera movements have been tested, the user can
fill out the test results (Succeess or Fail) and export the results to a Json file. 

How to use:

1. Download the resporitory
2. Open the project 
3. Run the project
4. Imput the correct IP address to connect to the websocket
5. Add your command - either using the dropdown or by inputting text, then click the add command button
if successful, a success message will pop up for 3 seconds - if fail, a fail message will pop up 
6. Click load to view a list of the commands 
7. Find the command you want to send to the camera and click the send button
8. Once you have ran your commands, fill out the testing form (choose success or fail)
9. Once the results have been filled out, click the export button
- A 1 = FAIL
- A 0 = SUCCESS
- A -1 = UNSELECTED
