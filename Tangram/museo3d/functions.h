#include <engine/utils.h>
#include <engine/variables.h>


//:::: CALLBACKS  Y FUNCIONES :::://
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void collidedObject_callback(string nameCollidedObject);
void collidedPhysicsObject_callback(string nameCollidedObject1, string nameCollidedObject2);
void joystick_callback(int jid, int event);
void processInput(GLFWwindow *window);
void rotateCamInput(double xoff, double yoff);
void actionKeys(GLFWwindow *window);

//NUEVO-------------------------
void setMultipleLight(Shader *shader, vector<glm::vec3> pointLightPositions);
void setSimpleLight(Shader *shader);
void loadAnimations(glm::mat4 view, glm::mat4 projection);
void drawModels(Shader *shader, glm::mat4 view, glm::mat4 projection);
void pickObjects(Shader *ourShader, Shader *selectShader, glm::mat4 projection);
void loadEnviroment(Terrain *terrain, SkyBox *sky, glm::mat4 view, glm::mat4 projection);
void physicsUpdate(Shader *shader);
void collisions();
bool allCanCollided();
bool fullFillObjectives();
void initScene(Shader ourShader);

//JOYSTICK CALLBACK
void joystick_callback(int jid, int event)
{

    if (event == GLFW_CONNECTED)
    {
        isJoyStick = true;
        cout << "Se ha conectado un control" << endl;
    }
    else if (event == GLFW_DISCONNECTED)
    {
        // The joystick was disconnected
        isJoyStick = false;
        cout << "Se ha desconectado el control" << endl;
    }
}
// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    GLFWgamepadstate state;
    // The joystick was connected
    if (glfwJoystickIsGamepad(GLFW_JOYSTICK_1))
    {
        isJoyStick = true;
        if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state))
        {

            if (state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y] == -1)
            {
                camera.ProcessKeyboard(FORWARD, deltaTime);
            }
            if (state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y] == 1)
            {
                camera.ProcessKeyboard(BACKWARD, deltaTime);
            }

            if (state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X] == 1)
            {
                xJoy += 50.5;
                rotateCamInput(xJoy, yJoy);
            }
            if (state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X] == -1)
            {
                xJoy -= 50.5;
                rotateCamInput(xJoy, yJoy);
            }
            if (state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y] == 1)
            {
                yJoy += 50.5;
                rotateCamInput(xJoy, yJoy);
            }
            if (state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y] == -1)
            {
                yJoy -= 50.5;
                rotateCamInput(xJoy, yJoy);
            }
        }
    }
    else
    {
        //MENÚS
        if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS)
        {
            int menu = (int)typemenu;
            if (menu < 5)
                menu += 1;
            else
                menu = 0;
            typemenu = (TypeActionKeyBoard)menu;
            system("cls");
            if (typemenu == GAME)
                cout << "\nMenu GAME seleccionado..." << endl;
            else if (typemenu == OBJECTS)
                cout << "\nMenu OBJECTS seleccionado..." << endl;
            else if (typemenu == COLLISION)
                cout << "\nMenu COLLISION seleccionado..." << endl;
            else if (typemenu == LIGHTING)
                cout << "\nMenu LIGHTING seleccionado..." << endl;
            else if (typemenu == PHYSICS)
                cout << "\nMenu PHYSICS seleccionado..." << endl;
        }

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            skyPos += camera.Front * glm::vec3(2);
            camera.ProcessKeyboard(FORWARD, deltaTime);
            colDetection = 25.0;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            camera.notFrontMove = false;
            camera.ProcessKeyboard(BACKWARD, deltaTime);
            skyPos -= camera.Front * glm::vec3(2);
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            skyPos -= camera.Right * glm::vec3(2);
            camera.ProcessKeyboard(LEFT, deltaTime);
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            skyPos -= camera.Right * glm::vec3(2);
            camera.ProcessKeyboard(RIGHT, deltaTime);
        }

        isJoyStick = false;
        actionKeys(window);
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
    }
}
void actionKeys(GLFWwindow *window)
{   
    if (typemenu == OBJECTS || typemenu == COLLISION  || typemenu == PHYSICS)
    {
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            if (movement < 0.2)
                movement += 0.01;
            else
                movement = 0.0;
            system("cls");
            cout << "\nEl factor de movimiento es: " << movement << endl;
        }
    }

    if (typemenu == PHYSICS)
    {

        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        {
            if (indexRigidModel < rigidModels.size() - 1)
            {
                indexRigidModel += 1;
                system("cls");
                cout << "\nModelo seleccionado: " << rigidModels[indexRigidModel].getModel().name << endl;
            }
            else
            {
                indexRigidModel = 0;
            }
        }

        //IMPRIMIR EN CONSOLA
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        {
            for (int i = 0; i < rigidModels.size(); i++)
            {
                rbRigidBody rb = *rigidModels[i].getRigidbox();
                rbVec3 mPos = rb.Position();
                float tx = mPos.x;
                float ty = mPos.y;
                float tz = mPos.z;
                cout << "\n rigidModels.push_back(RigidModel(\"" << rigidModels[i].getModel().name << "\", \"" << rigidModels[i].getModel().path << "\", glm::vec3(" << tx << ", " << ty << ", " << tz << "), 10.0, 0.0, 0.5, glm::vec3(0.08), rbRigidBody::Attribute_AutoSleep));" << endl;
            }
        }

        //EMPUJAR OBJETOS
        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
        {
            rbVec3 v(camera.Front.x, camera.Front.y, camera.Front.z);
            rbVec3 p(camera.Position.x, camera.Position.y, camera.Position.z);
            rigidModels[indexRigidModel].getRigidbox()->ResetStatuses();
            rigidModels[indexRigidModel].getRigidbox()->SetLinearVelocity(v * 0.5);
            rigidModels[indexRigidModel].getRigidbox()->SetForce(v * 0.5);
        }
        //RESETEAR OBJETOS CON FISICAS
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        {

            glm::vec3 pos = physicsObjectsPositions[indexRigidModel];
            rbVec3 p(pos.x, pos.y, pos.z);
            rigidModels[indexRigidModel].getRigidbox()->ResetStatuses();
            rigidModels[indexRigidModel].getRigidbox()->SetPosition(p);
            rigidModels[indexRigidModel].getRigidbox()->SetLinearVelocity(-p * 0.6);
            rigidModels[indexRigidModel].getRigidbox()->SetForce(-p * 0.6);
        }

        if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
        {

            rbVec3 v(camera.Front.x, camera.Front.y, camera.Front.z);
            rbVec3 p(camera.Position.x, camera.Position.y, camera.Position.z);
            rigidModels[indexRigidModel].getRigidbox()->ResetStatuses();
            rigidModels[indexRigidModel].getRigidbox()->SetPosition(v);
            rigidModels[indexRigidModel].getRigidbox()->SetLinearVelocity(v * 0.2);
            rigidModels[indexRigidModel].getRigidbox()->SetForce(v * 0.2);
        }

        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        {

            rbVec3 v(camera.Front.x, camera.Front.y, camera.Front.z);
            rbVec3 p(camera.Position.x, camera.Position.y, camera.Position.z);
            rigidModels[indexRigidModel].getRigidbox()->ResetStatuses();
            rigidModels[indexRigidModel].getRigidbox()->SetPosition(p);
            rigidModels[indexRigidModel].getRigidbox()->SetLinearVelocity(v * 10);
            rigidModels[indexRigidModel].getRigidbox()->SetForce(v * 10);
        }
    }
    else if (typemenu == GAME)
    {
        //RECOGER EXTINTOR
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        {
            pickObject = !pickObject;
            system("cls");
        }
        //GAME KEYS
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
            updateParticles = !updateParticles;

        if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
        {
            changeSkyBoxTexture = true;
            dianoche = !dianoche; //TOGGLE
        }
        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
            changeSprite = !changeSprite;

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            showMainMessage = !showMainMessage;
    }
    else if (typemenu == OBJECTS)
    {
        //::::::::::::::MOVER OBJETOS:::::::::::::://

        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        {
            if (indexObject < models.size() - 1)
            {
                indexObject += 1;
            }
            else
            {
                indexObject = 0;
            }

            system("cls");
            cout << "\nModelo seleccionado: " << models[indexObject].name << endl;
        }

        //TRANSLACIÓN DEL MODELO X
        if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
        {
            posModel = models[indexObject].getPosition();
            posModel.x -= movement;
            models[indexObject].setPosition(posModel);
        }
        if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        {
            posModel = models[indexObject].getPosition();
            posModel.x += movement;
            models[indexObject].setPosition(posModel);
        }
        //TRANSLACIÓN DEL MODELO Y
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        {
            posModel = models[indexObject].getPosition();
            posModel.y -= movement;
            models[indexObject].setPosition(posModel);
        }
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        {
            posModel = models[indexObject].getPosition();
            posModel.y += movement;
            models[indexObject].setPosition(posModel);
        }
        //TRANSLACIÓN DEL MODELO Z
        if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
        {
            posModel = models[indexObject].getPosition();
            posModel.z -= movement;
            models[indexObject].setPosition(posModel);
        }
        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
        {
            posModel = models[indexObject].getPosition();
            posModel.z += movement;
            models[indexObject].setPosition(posModel);
        }
        //ROTACIÓN DEL MODELO X
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
        {
            rotationModel = models[indexObject].getAngles();
            rotationModel.x -= 1.0;
            models[indexObject].setAngles(rotationModel);
        }
        if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
        {
            rotationModel = models[indexObject].getAngles();
            rotationModel.x += 1.0;
            models[indexObject].setAngles(rotationModel);
        }
        //ROTACIÓN DEL MODELO Y
        if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
        {
            rotationModel = models[indexObject].getAngles();
            rotationModel.y -= 1.0;
            models[indexObject].setAngles(rotationModel);
        }
        if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS)
        {
            rotationModel = models[indexObject].getAngles();
            rotationModel.y += 1.0;
            models[indexObject].setAngles(rotationModel);
        }
        //ROTACIÓN DEL MODELO Z
        if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS)
        {
            rotationModel = models[indexObject].getAngles();
            rotationModel.z -= 1.0;
            models[indexObject].setAngles(rotationModel);
        }
        if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS)
        {
            rotationModel = models[indexObject].getAngles();
            rotationModel.z += 1.0;
            models[indexObject].setAngles(rotationModel);
        }
        //IMPRIMIR EN CONSOLA
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        {
            cout << "\nfloat initScale = 0.2f;";
            for (int i = 0; i < models.size(); i++)
            {
                glm::vec3 mPos = models[i].getPosition();
                glm::vec3 mRot = models[i].getAngles();
                float tx = mPos.x;
                float ty = mPos.y;
                float tz = mPos.z;

                float rx = mRot.x;
                float ry = mRot.y;
                float rz = mRot.z;
                cout << "\nmodels.push_back(Model(\"" << models[i].name << "\",\"" << models[i].path << "\", glm::vec3(" << tx << ", " << ty << "," << tz << "), glm::vec3(" << rx << ", " << ry << "," << rz << "), 0.0f, initScale));" << endl;
            }
        }
    }
    else if (typemenu == LIGHTING)
    {
        //::::::::::::::MOVER LUCES:::::::::::::://
        //::::::::::::::MOVER LUCES:::::::::::::://
        if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
        {
            ejercicioIluminacion = !ejercicioIluminacion;
            system("cls");
            cout << "\nEl tipo de luz seleccionada es : " << ejercicioIluminacion << endl;
        }

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            const char *lightTypes[] = {"FLAT", "ALL", "DIRECTIONAL", "SPOT", "POINT"};
            int lt = (int)lightType;
            lt += 1;
            if (lt > 4)
                lt = 0;
            lightType = (LightType)lt;
            system("cls");
            cout << "\nEl tipo de luz seleccionada es : " << lightTypes[lt] << endl;
        }
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        {
            if (indexLight < pointLightPositions.size() - 1)
                indexLight += 1;
            else
                indexLight = 0;

            system("cls");
            cout << "\nLuz seleccionada: " << lightcubes.at(indexLight).first << endl;
        }
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        {
            renderLightingCubes = !renderLightingCubes;
            system("cls");
            if (renderLightingCubes)
                cout << "\nVisibilidad de light cubes activada..." << endl;
            else
                cout << "\nVisibilidad de light cubes desactivada..." << endl;
        }

        //TRANSLACIÓN DEL MODELO X
        if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
        {
            pointLightPositions[indexLight].x -= 0.5;
            lightcubes.at(indexLight).second.setPosition(pointLightPositions[indexLight]);
        }

        if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        {
            pointLightPositions[indexLight].x += 0.5;
            lightcubes.at(indexLight).second.setPosition(pointLightPositions[indexLight]);
        }

        //TRANSLACIÓN DEL MODELO Y
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        {
            pointLightPositions[indexLight].y -= 0.5;
            lightcubes.at(indexLight).second.setPosition(pointLightPositions[indexLight]);
        }

        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        {
            pointLightPositions[indexLight].y += 0.5;
            lightcubes.at(indexLight).second.setPosition(pointLightPositions[indexLight]);
        }

        //TRANSLACIÓN DEL MODELO Z
        if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
        {
            pointLightPositions[indexLight].z -= 0.5;
            lightcubes.at(indexLight).second.setPosition(pointLightPositions[indexLight]);
        }

        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
        {
            pointLightPositions[indexLight].z += 0.5;
            lightcubes.at(indexLight).second.setPosition(pointLightPositions[indexLight]);
        }

        //IMPRIMIR EN CONSOLA
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        {
            for (int i = 0; i < pointLightPositions.size(); i++)
            {
                glm::vec3 mPos = pointLightPositions[i];
                float tx = mPos.x;
                float ty = mPos.y;
                float tz = mPos.z;
                cout << "\nglm::vec3(" << tx << ", " << ty << "," << tz << ");" << endl;
            }
        }
    }
    else if (typemenu == COLLISION)
    {
        int size = isCollBoxModel ? models.size() : collboxes.size();
        glm::vec3 transform(0);
        //::::::::::::::MOVER COLLISION BOX:::::::::::::://
        //SWITCH ENTRE COLLBOXES Y MODELS
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        {
            isCollBoxModel = !isCollBoxModel;
            system("cls");
            if (isCollBoxModel)
            {
                indexCollBox = 0;
                transformObject = MODEL;
                cout << "\nSe ha seleccionado el movimiento de collbox de objetos..." << endl;
            }
            else
            {
                indexCollBox = 0;
                transformObject = COLLBOX;
                cout << "\nSe ha seleccionado el movimiento de collbox individuales..." << endl;
            }
        }
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        {
            renderCollBox = !renderCollBox;
            system("cls");
            if (renderCollBox)
                cout << "\nVisibilidad de collbox activada..." << endl;
            else
                cout << "\nVisibilidad de collbox desactivada..." << endl;
        }

        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        {
            if (indexCollBox < size)
            {
                string name = isCollBoxModel ? models[indexCollBox].name : collboxes.at(indexCollBox).first;
                cb = isCollBoxModel ? &models[indexCollBox].collbox : &collboxes.at(indexCollBox).second;
                system("cls");
                cout << "\nCollision Box seleccionado: " << name << endl;
                indexCollBox += 1;
            }
            else
                indexCollBox = 0;
        }
        //ESCALA DEL COLLBOX X
        if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS)
        {
            scaleCollBox = cb->getScale();
            scaleCollBox.x = (scaleCollBox.x < 0.5) ? 0.5 : scaleCollBox.x - movement;
            cb->setScale(scaleCollBox);
        }
        if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS)
        {
            scaleCollBox = cb->getScale();
            scaleCollBox.x += movement;
            cb->setScale(scaleCollBox);
        }
        //ESCALA DEL COLLBOX Y
        if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
        {
            scaleCollBox = cb->getScale();
            scaleCollBox.y = (scaleCollBox.y < 0.5) ? 0.5 : scaleCollBox.y - movement;
            cb->setScale(scaleCollBox);
        }
        if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS)
        {
            scaleCollBox = cb->getScale();
            scaleCollBox.y += movement;
            cb->setScale(scaleCollBox);
        }
        //ESCALA DEL COLLBOX Z
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
        {
            scaleCollBox = cb->getScale();
            scaleCollBox.z = (scaleCollBox.z < 0.5) ? 0.5 : scaleCollBox.z - movement;
            cb->setScale(scaleCollBox);
        }
        if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
        {
            scaleCollBox = cb->getScale();
            scaleCollBox.z += movement;
            cb->setScale(scaleCollBox);
        }
        //POSICIÓN DEL COLLBOX X
        if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
        {
            posModel = cb->getPosition();
            posModel.x -= movement;
            cb->setPosition(posModel);
        }
        if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        {
            posModel = cb->getPosition();
            posModel.x += movement;
            cb->setPosition(posModel);
        }
        //POSICIÓN DEL COLLBOX Y
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        {
            posModel = cb->getPosition();
            posModel.y -= movement;
            cb->setPosition(posModel);
        }
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        {
            posModel = cb->getPosition();
            posModel.y += movement;
            cb->setPosition(posModel);
        }
        //POSICIÓN DEL COLLBOX Z
        if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
        {
            posModel = cb->getPosition();
            posModel.z -= movement;
            cb->setPosition(posModel);
        }
        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
        {
            posModel = cb->getPosition();
            posModel.z += movement;
            cb->setPosition(posModel);
        }
        cb->defineCollisionBox();
        //IMPRIMIR EN CONSOLA
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        {
            cout << "\nCollisionBox collbox;" << endl;
           
            if (transformObject == COLLBOX)
            {
                int i = 0;
                for (pair<int, pair<string, CollisionBox>> colls : collboxes)
                {
                    string name = colls.second.first;
                    glm::vec3 mPos = colls.second.second.getPosition();
                    glm::vec3 mScale = colls.second.second.getScale();
                    cout << "\ncollbox = CollisionBox(glm::vec3(" << mPos.x << "," << mPos.y << "," << mPos.z << "), glm::vec3(" << mScale.x << "," << mScale.y << "," << mScale.z << "), glm::vec4(0.41f, 0.2f, 0.737f, 0.06f));"
                        << "\ncollboxes.insert(pair<int, pair<string, CollisionBox>>(" << i << ", pair<string, CollisionBox>(\"" << name << "\",collbox)));" << endl;
                    i++;
                }
            }
            else if (transformObject == MODEL)
            {
                for (int i=0; i<models.size(); i++)
                {
                    string name = models[i].name;
                    glm::vec3 mPos = models[i].collbox.getPosition();
                    glm::vec3 mScale = models[i].collbox.getScale();
                    cout << "\nmodels[" << i << "].collbox.setPosition(glm::vec3(" << mPos.x << "," << mPos.y << "," << mPos.z << "));"
                        << "\nmodels[" << i << "].collbox.setScale(glm::vec3(" << mScale.x << "," << mScale.y << "," << mScale.z << "));"
                        << "\nmodels[" << i << "].collbox.defineCollisionBox();" << endl;

                   
                }              
            }
            
           
        }
    }
}

void rotateCamInput(double xoff, double yoff)
{
    if (firstMouse)
    {
        lastX = xJoy;
        lastY = yJoy;
        firstMouse = false;
    }

    float xoffset = (xJoy - lastX) * 0.2;
    float yoffset = (lastY - yJoy) * 0.2; // reversed since y-coordinates go from bottom to top

    lastX = xJoy;
    lastY = yJoy;

    camera.ProcessMouseMovement(xoffset, yoffset);
}
// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (!isJoyStick)
    {

        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

        lastX = xpos;
        lastY = ypos;

        camera.ProcessMouseMovement(xoffset, yoffset);
    }
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    auto pos = camera.Position;
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
        updateParticles = true;
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

void collidedPhysicsObject_callback(string nameCollidedObject1, string nameCollidedObject2)
{
    renderCountPhysicsCollided = 30;
    textRendererPhysicsObjectsCollided = "Chocaron " + nameCollidedObject1 + " y " + nameCollidedObject2;
    //OBJETIVO 3 REVISAMOS SI CHOCARON TODAS LAS LATAS
    if (((nameCollidedObject1 == "mundet" || nameCollidedObject2 == "mundet") ||
         (nameCollidedObject1 == "pepsi" || nameCollidedObject2 == "pepsi") ||
         (nameCollidedObject1 == "sprite" || nameCollidedObject2 == "sprite") ||
         (nameCollidedObject1 == "coca" || nameCollidedObject2 == "coca")))
    {
        if (nameCollidedObject1 == "basura" || nameCollidedObject2 == "basura")
        {

            if (!cansCollided.getValue("mundet"))
                cansCollided.setValue("mundet", nameCollidedObject1 == "mundet" || nameCollidedObject2 == "mundet");

            if (!cansCollided.getValue("pepsi"))
                cansCollided.setValue("pepsi", nameCollidedObject1 == "pepsi" || nameCollidedObject2 == "pepsi");

            if (!cansCollided.getValue("coca"))
                cansCollided.setValue("coca", nameCollidedObject1 == "coca" || nameCollidedObject2 == "coca");

            if (!cansCollided.getValue("sprite"))
                cansCollided.setValue("sprite", nameCollidedObject1 == "sprite" || nameCollidedObject2 == "sprite");

            if (!objectives.getValue("lataspared"))
            {
                objectives.setValue("lataspared", allCanCollided());
                if (objectives.getValue("lataspared"))
                    fullFillObjectives();
            }
        }
    }

    //OBJETIVO 4 REVISAMOS SI SE HIZÓ CHUSA
    if (nameCollidedObject2 == "bolaboliche" && (nameCollidedObject1 == "mundet" ||
                                                 nameCollidedObject1 == "pepsi" ||
                                                 nameCollidedObject1 == "coca" ||
                                                 nameCollidedObject1 == "sprite"))
    {
        if (!objectives.getValue("chusaLatas"))
        {

            objectives.setValue("chusaLatas", true);
            fullFillObjectives();
        }
    }
}
void collidedObject_callback(string nameCollidedObject)
{
    const char *text = "";
    //NUEVO
    if (nameCollidedObject == "extintor")
    {
        isExtinguisherCollided = true;
        text = "Extinguidor encontrado";
        showSecondMessage = true;
        Text->RenderText(text, -0.55f, 0.7f, 0.002f, glm::vec3(1.0f, 1.0f, 0.0f));
        secondMessage->Draw(glm::vec2(0.0f, -0.75f), 1.0f);
    }
    else
        isExtinguisherCollided = false;

    //NUEVO
    if (nameCollidedObject == "pedido")
    {
        text = "Pedido Encontrado";
        showSecondMessage = true;
        Text->RenderText(text, -0.55f, 0.7f, 0.002f, glm::vec3(1.0f, 1.0f, 0.0f));
        secondMessage->Draw(glm::vec2(0.0f, -0.75f), 1.0f);
        isBoxCollided = true;
        if (!objectives.getValue("recogerPedido"))
        {
            objectives.setValue("recogerPedido", pickObject);
            if (objectives.getValue("recogerPedido"))
                fullFillObjectives();
        }
    }
    else
        isBoxCollided = false;

    if (nameCollidedObject == "carrorojo")
    {
        text = "Punto de Encuentro";
        showSecondMessage = true;
        Text->RenderText(text, -0.55f, 0.7f, 0.002f, glm::vec3(1.0f, 1.0f, 0.0f));
        secondMessage->Draw(glm::vec2(0.0f, -0.75f), 1.0f);
        if (!objectives.getValue("dejarPedido"))
        {
            objectives.setValue("dejarPedido", (pickObject == false && intersect(models[19].collbox, pickModels[2].collbox)));
            if (objectives.getValue("dejarPedido"))
                fullFillObjectives();
        }
    }

    if (nameCollidedObject == "fuego")
    {
        if (!objectives.getValue("apagarFuego"))
        {
            objectives.setValue("apagarFuego", (!pickObject && intersect(collboxes[4].second, pickModels[0].collbox)));
            if (objectives.getValue("apagarFuego"))
                fullFillObjectives();
        }
    }

    if (nameCollidedObject == "puertaizq" || nameCollidedObject == "puertader")
    {
        doorAnim = true;
        openDoor = true;
    }

    if (nameCollidedObject == "e1")
    {
        text = " Carnes frias";
        showSecondMessage = true;
        Text->RenderText(text, -0.55f, 0.7f, 0.002f, glm::vec3(1.0f, 1.0f, 0.0f));
        secondMessage->Draw(glm::vec2(0.0f, -0.75f), 1.0f);
    }

    else if (nameCollidedObject == "e2")
    {

        text = " Cereales";
        Text->RenderText(text, -0.4f, 0.7f, 0.002f, glm::vec3(1.0f, 1.0f, 0.0f));
        secondMessage->Draw(glm::vec2(0.0f, -0.75f), 1.0f);
    }
    else if (nameCollidedObject == "e3")
    {
        text = " Bebidas";
        showSecondMessage = true;
        Text->RenderText(text, -0.35f, 0.7f, 0.002f, glm::vec3(1.0f, 1.0f, 0.0f));
        secondMessage->Draw(glm::vec2(0.0f, -0.75f), 1.0f);
    }
    else
    {
        showSecondMessage = false;
    }
}