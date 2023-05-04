#include <GLFW/glfw3.h>
#include <engine/Billboard.h>
#include <engine/CollisionBox.h>
#include <engine/Objectives.h>
#include <engine/Particles.h>
#include <engine/Plane.h>
#include <engine/QuadTexture.h>
#include <engine/RigidModel.h>
#include <engine/Terrain.h>
#include <engine/functions.h>
#include <engine/shader_m.h>
#include <engine/skybox.h>
#include <engine/textrenderer.h>
#include <glad/glad.h>
#include <iostream>

int main()
{
    //:::: INICIALIZAMOS GLFW CON LA VERSIÓN 3.3 :::://
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    //:::: CREAMOS LA VENTANA:::://
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "THE HOSPITAL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    //:::: OBTENEMOS INFORMACIÓN DE TODOS LOS EVENTOS DE LA VENTANA:::://
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetJoystickCallback(joystick_callback);

    //:::: DESHABILITAMOS EL CURSOR VISUALMENTE EN LA PANTALLA :::://
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    //:::: INICIALIZAMOS GLAD CON LAS CARACTERÍSTICAS DE OPENGL ESCENCIALES :::://
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    //INICIALIZAMOS LA ESCENA
    Shader ourShader("shaders/multiple_lighting.vs", "shaders/multiple_lighting.fs");
    Shader selectShader("shaders/selectedShader.vs", "shaders/lighting_maps.fs");
    initScene(ourShader);
    Terrain terrain("textures/heightmap3.jpg", texturePaths);
    SkyBox sky(1.0f, "6");
    cb = isCollBoxModel ? &models[indexCollBox].collbox : &collboxes.at(indexCollBox).second;

    //:::: RENDER:::://
    while (!glfwWindowShouldClose(window))
    {

        glfwSetWindowTitle(window, ("X: " + std::to_string(camera.Position.x)
            + " Y: " + std::to_string(camera.Position.y) + " Z: " + std::to_string(camera.Position.z)).c_str());

        //::::TIMING:::://Ayuda a crear animaciones fluidas
        float currentFrame = glfwGetTime();
        deltaTime = (currentFrame - lastFrame);
        lastFrame = currentFrame;
        respawnCount += 0.1;

        //::::ENTRADA CONTROL:::://
        processInput(window);
        //:::: LIMPIAMOS BUFFERS:::://
        glClearColor(0.933f, 0.811f, 0.647f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //:::: PASAMOS INFORMACIÓN AL SHADER:::://
        ourShader.use();

        //:::: DEFINICIÓN DE MATRICES::::// La multiplicaciónd e model*view*projection crea nuestro entorno 3D
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                                (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        //:::: FISICAS:::://
        physicsUpdate(&ourShader);
        //:::: RENDER DE MODELOS:::://
        drawModels(&ourShader, view, projection);
        //:::: SKYBOX Y TERRENO:::://
        loadEnviroment(&terrain, &sky, view, projection);
        //:::: COLISIONES:::://
        collisions();
        //:::: COLISIONES:::://
        pickObjects(&ourShader, &selectShader, projection);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    //:::: LIBERACIÓN DE MEMORIA:::://
    particles.Release();
    fireAnimBillboard.Release();
    clockAnimBillboard.Release();
    humoAnimBillboard.Release();
    relojAnimBillboard.Release();
    explosionAnimBillboard.Release();
    cafeAnimBillboard.Release();
    qt.Release();
    qt2.Release();
    qt3.Release();
    delete[] texturePaths;
    sky.Release();
    terrain.Release();
    for (int i = 0; i < lightcubes.size(); i++)
        lightcubes[i].second.Release();
    for (int i = 0; i < collboxes.size(); i++)
        collboxes[i].second.Release();
    for (int i = 0; i < models.size(); i++)
        models[i].Release();
    for (int i = 0; i < rigidModels.size(); i++)
    {
        physicsEnviroment.Unregister(rigidModels[i].getRigidbox());
    }
    physicsEnviroment.Unregister(&piso);
    physicsEnviroment.Unregister(&pared);
    plane.Release();
    glfwTerminate();

    return 0;
}

void initScene(Shader ourShader)
{

    renderCollBox = true;
    renderLightingCubes = true;

    //AGUA
    //:::: DEFINIMOS LAS TEXTURAS DE LA MULTITEXTURA DEL TERRENO :::://
    texturePaths = new const char *[4];
    texturePaths[0] = "textures/multitexture_colors.jpg";
    texturePaths[1] = "textures/pasto.jpg";
    texturePaths[2] = "textures/rock.png";
    texturePaths[3] = "textures/arena.jpg";

    //:::: POSICIONES DE TODAS LAS LUCES :::://
    pointLightPositions.push_back(glm::vec3(2.3f, 5.2f, 2.0f));
    pointLightPositions.push_back(glm::vec3(2.3f, 10.3f, -4.0f));
    pointLightPositions.push_back(glm::vec3(-4.0f, 10.0f, -12.0f));
    pointLightPositions.push_back(glm::vec3(0.0f, 10.0f, -3.0f));

    //:::: POSICIONES DE TODAS LAS LUCES PARA EL EJERCICIO:::://
    //objetos
    ejercicioIlumPositions.push_back(glm::vec3(-25, 4.8, 10));
    ejercicioIlumPositions.push_back(glm::vec3(-30, 4.8, 10));
    ejercicioIlumPositions.push_back(glm::vec3(-30, 4.8, 20));

    //pedestales
    ejercicioIlumPositions.push_back(glm::vec3(-25, 2, 10));
    ejercicioIlumPositions.push_back(glm::vec3(-30, 2, 10));
    ejercicioIlumPositions.push_back(glm::vec3(-30, 2, 20));

    //point
    ejercicioIlumPositions.push_back(glm::vec3(-25, 7, 10));
    ejercicioIlumPositions.push_back(glm::vec3(-30, 7, 10));
    ejercicioIlumPositions.push_back(glm::vec3(-30, 7, 20));

    //:::: POSICIONES DE TODOS LOS OBJETOS CON FISICAS :::://
    physicsObjectsPositions.push_back(glm::vec3(0.0, 10.0, 0.0));
    physicsObjectsPositions.push_back(glm::vec3(2.0, 10.0, 0.0));
    physicsObjectsPositions.push_back(glm::vec3(1.0, 10.0, 0.0));
    physicsObjectsPositions.push_back(glm::vec3(-2.0, 10.0, -2.0));
    physicsObjectsPositions.push_back(glm::vec3(-2.0, 10.0, -2.0));
    physicsObjectsPositions.push_back(glm::vec3(15.0, 1.0, -2.0));
    physicsObjectsPositions.push_back(glm::vec3(15.0, 1.0, -2.0));
    physicsObjectsPositions.push_back(glm::vec3(25.0, 10.0, -2.0));

    //:::: OBJETIVOS :::://
    objectives.AddObjective("recogerPedido", false);
    objectives.AddObjective("dejarPedido", false);
    objectives.AddObjective("lataspared", false);
    objectives.AddObjective("chusaLatas", false);
    objectives.AddObjective("apagarFuego", false);

    cansCollided.AddObjective("mundet", false);
    cansCollided.AddObjective("pepsi", false);
    cansCollided.AddObjective("sprite", false);
    cansCollided.AddObjective("coca", false);

    //:::: CONFIGURAMOS LOS VALORES DE MOVIMIENTO DE LAS PARTICULAS :::://
    m_Particle.ColorBegin = {254 / 255.0f, 212 / 255.0f, 123 / 255.0f, 1.0f};
    m_Particle.ColorEnd = {254 / 255.0f, 109 / 255.0f, 41 / 255.0f, 1.0f};
    m_Particle.SizeVariation = 0.3f;
    m_Particle.SizeBegin = 0.5f;
    m_Particle.SizeEnd = 0.0f;
    m_Particle.LifeTime = 1.0f;
    m_Particle.Velocity = {0.2f, 1.0f, 0.5f};
    m_Particle.VelocityVariation = {0.2f, 1.0f, 1.0f};
    m_Particle.Position = {0.0f, 0.0f, 0.0f};

    //:::: ESTADO GLOBAL DE OPENGL :::://
    glEnable(GL_DEPTH_TEST);

    //Definimos los collbox de la camara
    camera.setCollBox();
    //:::: CARGAMOS LOS SHADERS :::://

    //:::: IMPLEMENTAMOS EL SHADER PARA SU CONFIGURACIÓN :::://
    ourShader.use();

    //:::: INICIALIZAMOS NUESTROS OBJETOS :::://
    //:::: PARTICULAS:::://

    //:::: BILLBOARDS :::://
    particles = Particles("textures/fire2.png");
    fireAnimBillboard = Billboard("textures/spritesfire.png", (float)SCR_WIDTH, (float)SCR_HEIGHT, 64.0f, 64.0f);
    clockAnimBillboard = Billboard("textures/reloj.png", (float)SCR_WIDTH, (float)SCR_HEIGHT, 140.0f, 140.0f);
    humoAnimBillboard = Billboard("textures/Humo2.png", (float)SCR_WIDTH, (float)SCR_HEIGHT, 140.0f, 140.0f);
    relojAnimBillboard = Billboard("textures/Reloj1.png", (float)SCR_WIDTH, (float)SCR_HEIGHT, 93.0f, 93.0f);
    explosionAnimBillboard = Billboard("textures/Explosion.png", (float)SCR_WIDTH, (float)SCR_HEIGHT, 140.0f, 140.0f);
    cafeAnimBillboard = Billboard("textures/Cafe1.png", (float)SCR_WIDTH, (float)SCR_HEIGHT, 115.0f, 115.0f);
    fireAnimBillboard.setPosition(glm::vec3(-4.0f, -4.5f, 0.0f));
    clockAnimBillboard.setPosition(glm::vec3(0.0f, -3.5f, 0.0f));
    humoAnimBillboard.setPosition(glm::vec3(20.22, 3, -15));
    explosionAnimBillboard.setPosition(glm::vec3(13, 3, -10));//4.0f, 0.2f, 0.0f
    relojAnimBillboard.setPosition(glm::vec3(17, 3, -7));//1.0f, 0.2f, 0.0f
    cafeAnimBillboard.setPosition(glm::vec3(15, 3, -19.1)); //-2.0f, 0.2f, 0.0f
    fireAnimBillboard.setScale(1.5f);
    explosionAnimBillboard.setScale(0.7f);
    relojAnimBillboard.setScale(0.7f);
    clockAnimBillboard.setScale(0.7f);
    humoAnimBillboard.setScale(0.7f);
    cafeAnimBillboard.setScale(0.7f);

    //:::: GUI:::://
    secondMessage = new QuadTexture("textures/containers.png", 240.0f, 240.0f, 4, 0);
    Text = new TextRenderer(SCR_WIDTH, SCR_HEIGHT);
    Text = new TextRenderer(SCR_WIDTH, SCR_HEIGHT);
    Text->Load("fonts/OCRAEXT.TTF", 60);
    qt = QuadTexture("textures/coin.png", 65.0f, 65.0f, 0, 0);
    qt2 = QuadTexture("textures/coin.png", 65.0f, 65.0f, 0, 0);
    qt3 = QuadTexture("textures/coin.png", 65.0f, 65.0f, 0, 0);
    mainMessage = QuadTexture("textures/containers.png", 240.0f, 240.0f, 0, 0);
    carIcon = QuadTexture("textures/items.png", 98.0f, 98.0f, 6, 0);
    p1 = QuadTexture("textures/items.png", 98.0f, 98.0f, 0, 0);
    p2 = QuadTexture("textures/items.png", 98.0f, 98.0f, 0, 0);
    p3 = QuadTexture("textures/items.png", 98.0f, 98.0f, 0, 0);

    //:::: INICIALIZAMOS NUESTROS MODELOS :::://
    cruce = Model("cruce", "models/objects/cruce.obj", glm::vec3(3.4, -10, -2), glm::vec3(0, 0, 0), 0.0f, initScale, false);
    
    models.push_back(Model("maquinasodas2", "models/objects/maquinaSodas.obj", glm::vec3(25.89, -5, 7.67001), glm::vec3(0, -275, 0), 0.0f, 0.25));
    models.push_back(Model("seven", "models/objects/seven.obj", glm::vec3(18.0701, -9, 11.2801), glm::vec3(0, -180, 0), 0.0f, 0.4, false));
    models.push_back(Model("maquinasodas", "models/objects/maquinaSodas.obj", glm::vec3(25.9501, -5, 6.21), glm::vec3(0, -275, 0), 0.0f, 0.25));
    models.push_back(Model("puertaizq", "models/objects/puerta_izquierda.obj", glm::vec3(10.42, -5, 15.6802), glm::vec3(0, 1, 0), 0.0f, 0.4));
    models.push_back(Model("puertader", "models/objects/puerta_izquierda.obj", glm::vec3(10.44, -5, 13.99), glm::vec3(0, -180, 0), 0.0f, 0.4));
    models.push_back(Model("e1", "models/Estante1.obj", glm::vec3(21.6, -5, 11.6), glm::vec3(0, 0, 0), 0.0f, 0.2));
    models.push_back(Model("e2", "models/Estante2.obj", glm::vec3(19.6, -5, 11.6), glm::vec3(0, 0, 0), 0.0f, 0.2));
    models.push_back(Model("e3", "models/Estante1.obj", glm::vec3(17.6, -5, 11.6), glm::vec3(0, 0, 0), 0.0f, 0.2));
    models.push_back(Model("e4", "models/Estante1.obj", glm::vec3(15.6, -5, 11.6), glm::vec3(0, 0, 0), 0.0f, 0.2));
    models.push_back(Model("e5", "models/Estante3.obj", glm::vec3(21.6, -5, 10.14), glm::vec3(0, -180, 0), 0.0f, 0.2));
    models.push_back(Model("e6", "models/Estante2.obj", glm::vec3(19.6, -5, 10.14), glm::vec3(0, -180, 0), 0.0f, 0.2));
    models.push_back(Model("e7", "models/Estante2.obj", glm::vec3(17.6, -5, 10.14), glm::vec3(0, -180, 0), 0.0f, 0.2));
    models.push_back(Model("e8", "models/Estante3.obj", glm::vec3(15.6, -5, 10.14), glm::vec3(0, -180, 0), 0.0f, 0.2));
    models.push_back(Model("estantemediano", "models/objects/estante_mediano.obj", glm::vec3(22.3701, -5, 16.68), glm::vec3(0, 275, 2), 0.0f, 0.2));
    models.push_back(Model("estantemediano2", "models/objects/estante_mediano.obj", glm::vec3(24.02, -5, 16.87), glm::vec3(0, -90, 2), 0.0f, 0.2));
    models.push_back(Model("estantegrande", "models/objects/estanteGrande.obj", glm::vec3(18.5, -5, 16.48), glm::vec3(0, 0, 0), 0.0f, 0.2));
    models.push_back(Model("estantegrande2", "models/objects/estanteGrande.obj", glm::vec3(15.55, -5, 16.44), glm::vec3(0, 0, 0), 0.0f, 0.2));
    models.push_back(Model("cajaregistradora2", "models/objects/cajaregistradora.obj", glm::vec3(15.5401, -5, 6.52), glm::vec3(0, 0, 0), 0.0f, 0.25));
    models.push_back(Model("cajaregistradora", "models/objects/cajaregistradora.obj", glm::vec3(12.78, 0-5, 6.42), glm::vec3(0, 0, 0), 0.0f, 0.25));
    models.push_back(Model("carrorojo", "models/objects/CarroRojo.obj", glm::vec3(5.3, -5, -4.3), glm::vec3(0, 90, 0), 0.0f, initScale));
    models.push_back(Model("señal1", "models/objects/señal1.obj", glm::vec3(8.1, -5, -6.7), glm::vec3(0, -90, 0), 0.0f, initScale));
    models.push_back(Model("señal2", "models/objects/señal2.obj", glm::vec3(-1.7, -5, 3.3), glm::vec3(0, -90, 0), 0.0f, initScale));
    models.push_back(Model("señal3", "models/objects/señal3.obj", glm::vec3(-1.7, -5, -7.49999), glm::vec3(0, 1, 0), 0.0f, initScale));
    models.push_back(Model("carroazul", "models/objects/CarroAzul.obj", glm::vec3(-9.6, -5, -2), glm::vec3(0, 0, 0), 0.0f, initScale));
    models.push_back(Model("van", "models/objects/Van.obj", glm::vec3(12, -5, -4.5), glm::vec3(0, 90, 0), 0.0f, initScale));
    

    //MIOS
    models.push_back(Model("camahospital", "models/objects/CamaHospital1.obj", glm::vec3(-30.2, 2.9, -28), glm::vec3(0, 180, 0), 0.0f, 0.01));
    models.push_back(Model("silla", "models/objects/SillaHospital.obj", glm::vec3(-23.4, 2.6, -24.6), glm::vec3(0, 90, 0), 0.0f, 0.01));
    models.push_back(Model("puertaH", "models/objects/PuertaHospital.obj", glm::vec3(-32.8, -1, -18.6), glm::vec3(0, 90, 0), 0.0f, 0.001));
    models.push_back(Model("corrido", "models/objects/curtain.obj", glm::vec3(-23, 10.8, -23), glm::vec3(0, 90, 0), 0.0f, 1));
    models.push_back(Model("vitrinas", "models/objects/VitrinasHospital1.obj", glm::vec3(-32.5, 10.8, -19), glm::vec3(0, 90, 0), 0.0f, 0.1));
    models.push_back(Model("incubador", "models/objects/incubador.obj", glm::vec3(-27, 6.6, -29.9), glm::vec3(0, 0, 0), 0.0f, 0.1));
    models.push_back(Model("sideboard", "models/objects/sideboard1.obj", glm::vec3(-28, 2.7, -17), glm::vec3(0, -90, 0), 0.0f, 0.009));
    models.push_back(Model("succionador", "models/objects/Succionador.obj", glm::vec3(-19, 4.4, -24), glm::vec3(0, 90, 0), 0.0f, 0.01));
    models.push_back(Model("cuchillo", "models/objects/cuchillo.obj", glm::vec3(-17.4, 2.6, -28.3), glm::vec3(0, 90, 0), 0.0f, 0.1));
    models.push_back(Model("cagua", "models/objects/Cagua1.obj", glm::vec3(-18.5, -1.6, -16), glm::vec3(0, 0, 0), 0.0f, 0.1));
    models.push_back(Model("PartesH", "models/objects/PartesC.obj", glm::vec3(-18, 6.5, -19), glm::vec3(0, 90, 0), 0.0f, 0.2));
    /*models.push_back(Model("Mbuilding", "models/objects/Hospital.obj", glm::vec3(-5, 2, -10), glm::vec3(0, -180, 0), 0.0f, 0.4, false));*/
    //models.push_back(Model("casaPrincipal", "models/objects/CasaPrincipal2.obj", glm::vec3(-22, -0.5, -3.7), glm::vec3(0, -90, 0), 0.0f, 0.001));
    models.push_back(Model("Mbuilding", "models/objects/Hospital2.obj", glm::vec3(-25, -1.5, -20), glm::vec3(0, 0, 0), 0.0f, 0.2));
    models.push_back(Model("CasaModerna", "models/objects/Casa_Japonesa.obj", glm::vec3(8.2, -1.8, 1), glm::vec3(0, 90, 0), 0.0f, 8));

    modelsIlumination.push_back(Model("Periodico", "models/objects/Periodico.obj", ejercicioIlumPositions[0], glm::vec3(0, 90, 0), 0.0f, 0.1));
    modelsIlumination.push_back(Model("Kamen", "models/objects/incubador.obj", ejercicioIlumPositions[1], glm::vec3(0, 90, 0), 0.0f, 0.1));
    modelsIlumination.push_back(Model("ArchivoSe", "models/objects/PeriodicoSecre.obj", ejercicioIlumPositions[2], glm::vec3(0, 90, 0), 0.0f, 0.04));
    modelsIlumination.push_back(Model("Pedestal1", "models/objects/pedestal.obj", ejercicioIlumPositions[3], glm::vec3(0, 90, 0), 0.0f, 0.2));
    modelsIlumination.push_back(Model("Pedestal2", "models/objects/pedestal.obj", ejercicioIlumPositions[4], glm::vec3(0, 90, 0), 0.0f, 0.2));
    modelsIlumination.push_back(Model("Pedestal3", "models/objects/pedestal.obj", ejercicioIlumPositions[5], glm::vec3(0, 90, 0), 0.0f, 0.2));

    //AQUÍ

    pickModels.push_back(Model("extintor", "models/fire extinguisher.obj", glm::vec3(11.26, -7, 6.9), glm::vec3(0, 0, 0), 0.0f, initScale));
    pickModels.push_back(Model("extintor2", "models/fire extinguisher.obj", glm::vec3(0.7f, -1.8f, -0.4f), glm::vec3(0, -45, 23), 0.0f, 0.8));
    pickModels.push_back(Model("pedido", "models/pedido.obj", glm::vec3(15.9401, -7, 6.52), glm::vec3(0, 91, 0), 0.0f, 0.6));
    pickModels.push_back(Model("pedido2", "models/pedido.obj", glm::vec3(0.55f, -1.5f, -0.5f), glm::vec3(0, 0, 0), 0.0f, 2.0));

    //CREAMOS TODAS  LAS CAJAS DE COLISION INDIVIDUALES
    CollisionBox collbox;
    glm::vec4 colorCollbox(0.41f, 0.2f, 0.737f, 0.06f);
    collbox = CollisionBox(glm::vec3(25.97, -10, 11), glm::vec3(0.3, 5, 12.4), colorCollbox);
    collboxes.insert(pair<int, pair<string, CollisionBox>>(0, pair<string, CollisionBox>("pared_frente_izq", collbox)));
    collbox = CollisionBox(glm::vec3(9.88, -10, 7.45999), glm::vec3(0.3, 4.6, 7.6), colorCollbox);
    collboxes.insert(pair<int, pair<string, CollisionBox>>(1, pair<string, CollisionBox>("pared_frente_der", collbox)));
    collbox = CollisionBox(glm::vec3(10.37, -10, 18.87), glm::vec3(0.3, 5.4, 1.2), colorCollbox);
    collboxes.insert(pair<int, pair<string, CollisionBox>>(2, pair<string, CollisionBox>("pared_frente_arriba", collbox)));
    collbox = CollisionBox(glm::vec3(10.35, -10, 14.85), glm::vec3(0.3, 1, 3.6), colorCollbox);
    collboxes.insert(pair<int, pair<string, CollisionBox>>(3, pair<string, CollisionBox>("techo", collbox)));
    collbox = CollisionBox(glm::vec3(-4.0f, -4.5f, 0.0f), glm::vec3(0.5), colorCollbox);
    collboxes.insert(pair<int, pair<string, CollisionBox>>(4, pair<string, CollisionBox>("fuego", collbox)));

    //CREAMOS LOS LIGHTCUBES QUE ENREALIDAD SON COLLISION BOXES QUE NOS AYUDARAN A IDENTIFICAR LA POSICIÓN DE DONDE ESTAN
    glm::vec3 lScale(0.5);
    colorCollbox = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    collbox = CollisionBox(pointLightPositions[0], lScale, colorCollbox);
    lightcubes.insert(pair<int, pair<string, CollisionBox>>(0, pair<string, CollisionBox>("LUZ1", collbox)));
    collbox = CollisionBox(pointLightPositions[1], lScale, colorCollbox);
    lightcubes.insert(pair<int, pair<string, CollisionBox>>(1, pair<string, CollisionBox>("LUZ2", collbox)));
    collbox = CollisionBox(pointLightPositions[2], lScale, colorCollbox);
    lightcubes.insert(pair<int, pair<string, CollisionBox>>(2, pair<string, CollisionBox>("LUZ3", collbox)));
    collbox = CollisionBox(pointLightPositions[3], lScale, colorCollbox);
    lightcubes.insert(pair<int, pair<string, CollisionBox>>(3, pair<string, CollisionBox>("LUZ4", collbox)));

    //:::: FISICAS :::://
    //AÑADIMOS TODOS LOS MODELOS DEL TIPO RIGID MODEL
    rigidModels.push_back(RigidModel("carrito", "models/carrito.obj", physicsObjectsPositions[0], 50.0, 0.0, 0.5, glm::vec3(0.08), rbRigidBody::Attribute_AutoSleep));
    rigidModels.push_back(RigidModel("sprite", "models/sprite.obj", physicsObjectsPositions[1], 10.0, 0.0, 0.5, glm::vec3(0.12), rbRigidBody::Attribute_AutoSleep));
    rigidModels.push_back(RigidModel("coca", "models/coca.obj", physicsObjectsPositions[2], 10.0, 0.0, 0.5, glm::vec3(0.12), rbRigidBody::Attribute_AutoSleep));
    rigidModels.push_back(RigidModel("mundet", "models/mundet.obj", physicsObjectsPositions[3], 10.0, 0.0, 0.5, glm::vec3(0.12), rbRigidBody::Attribute_AutoSleep));
    rigidModels.push_back(RigidModel("pepsi", "models/pepsi.obj", physicsObjectsPositions[4], 10.0, 0.0, 0.5, glm::vec3(0.12), rbRigidBody::Attribute_AutoSleep));
    rigidModels.push_back(RigidModel("basura", "models/basura.obj", glm::vec3(4.22502, 0.1, 3.80488), 10.0, 0.0, 0.5, glm::vec3(0.2), rbRigidBody::Attribute_AutoSleep));
    rigidModels.push_back(RigidModel("bolsabasura", "models/trashbag.obj", glm::vec3(4.54844, 0.1, 4.37938), 10.0, 0.0, 0.5, glm::vec3(0.22), rbRigidBody::Attribute_AutoSleep));
    rigidModels.push_back(RigidModel("bolaboliche", "models/bowlingBall.obj", physicsObjectsPositions[7], 10.0, 0.0, 0.5, glm::vec3(0.2), rbRigidBody::Attribute_AutoSleep));

    //QUITAMOS DEL ARREGLO PRINCIPAL EL BOTE DE BASURA PARA QUE NO COLISIONE CONSIGO MISMO
    rbmodels = rigidModels;
    rbmodels.erase(rbmodels.begin() + 4, rbmodels.end());

    //DEFINIMOS LOS RIGID BODY (SON CUERPOS QUE LES AFECTA LA FISICA Y COLISIONAN ENTRE SI)
    rbEnvironment::Config config;
    config.RigidBodyCapacity = 100;              //CANTIDAD DE RIGID BODIES
    config.ContactCapacty = 100;                 //CANTIDAD DE CONTACTOS ENTRE LSO RIGID BODIES
    piso.SetShapeParameter(10000.0f,             //MASA
                           100.0f, 0.5f, 100.0f, //TAMAÑO
                           0.1f, 0.3f);          //COEF RESTITUCIÓN, COEF FRICCIÓN
    piso.SetPosition(rbVec3(0.0, -0.35, 0.0));
    piso.EnableAttribute(rbRigidBody::Attribute_AutoSleep); //SERÁ FIJO NO LE AFECTARÁ LA FISICA SOLO  SERVIRÁ PARA COLISIONAR
    piso.name = "piso";
    pared.SetShapeParameter(10.0f,
                            0.3, 10, 6.4,
                            0.0f, 0.5f);
    pared.SetPosition(rbVec3(5, -0.3, 0.0));
    pared.EnableAttribute(rbRigidBody::Attribute_Fixed);
    pared.name = "pared";
    //AÑADIMOS AL AMBIENTE DE FISICAS LOS RIGIDBODIES QUE CREAMOS PARA QUE EL SISTEMA HAGA QUE COLISIONEN ENTRE SI
    physicsEnviroment = rbEnvironment(config);
    physicsEnviroment.Register(&piso);
    physicsEnviroment.Register(&pared);
    for (int i = 0; i < rigidModels.size(); i++)
        physicsEnviroment.Register(rigidModels[i].getRigidbox());

    //:::: AMBIENTE:::://

    plane = Plane("textures/watter.jpg", 100.0, 100.0, 0.0, 0.0);

    plane.setPosition(glm::vec3(26.9563618, -0.9, -35));
    plane.setAngles(glm::vec3(90.0, 0.0, 0.0));
    plane.setScale(glm::vec3(50.0));
}
//:::: CONFIGURACIONES :::://
void pickObjects(Shader *ourShader, Shader *selectShader, glm::mat4 projection)
{
    //SI COLISIONO DIBUJAMOS EL EXTINTOR QUE USA EL SHADER SELECTSHADER.VS
    glm::vec3 m = camera.Position + camera.Front * 1.2f; //UBICAMOS AL FRENTE DE LA CAMARA AL EXTINTOR
    if (pickObject && isExtinguisherCollided)
    {
        selectShader->use();
        setSimpleLight(selectShader);
        pickModels[1]
            .Draw(*selectShader);
        pickModels[0]
            .setPosition(glm::vec3(m.x, 0.2, m.z));
        selectShader->notUse();
    }
    else
    {
        ourShader->use();
        pickModels[0].Draw(*ourShader);
        ourShader->notUse();
    }

    if (pickObject && isBoxCollided)
    {
        selectShader->use();
        setSimpleLight(selectShader);
        pickModels[3]
            .Draw(*selectShader);
        pickModels[2].setPosition(glm::vec3(m.x, 0.2, m.z));
        selectShader->notUse();
    }
    else
    {
        ourShader->use();
        pickModels[2].Draw(*ourShader);
        ourShader->notUse();
    }
}
void loadEnviroment(Terrain *terrain, SkyBox *sky, glm::mat4 view, glm::mat4 projection)
{
    glm::mat4 model = mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0, -2.5f, 0.0f));   // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(100.5f, 100.0f, 100.5f)); // it's a bit too big for our scene, so scale it down

    terrain->draw(model, view, projection);
    terrain->getShader()->setFloat("shininess", 100.0f);
    setMultipleLight(terrain->getShader(), pointLightPositions);

    glm::mat4 skyModel = mat4(1.0f);
    skyModel = glm::translate(skyModel, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
    skyModel = glm::scale(skyModel, glm::vec3(40.0f, 40.0f, 40.0f));  // it's a bit too big for our scene, so scale it down
    sky->draw(skyModel, view, projection, skyPos);
    sky->getShader()->setFloat("shininess", 10.0f);
    setMultipleLight(sky->getShader(), pointLightPositions);

    loadAnimations(view, projection);
    //RENDER DE LIGHT CUBES
    if (renderLightingCubes)
        for (pair<int, pair<string, CollisionBox>> lights : lightcubes)
            lights.second.second.draw(view, projection);

    //CAMBIO DÍA NOCHE
    if (changeSkyBoxTexture)
    {
        if (dianoche)
        {
            mainLight = vec3(0.5);
            sky->reloadTexture("6");
        }
        else
        {
            mainLight = vec3(0.2);
            sky->reloadTexture("5");
        }
        changeSkyBoxTexture = false;
    }
}
void collisions()
{

    detectColls(collboxes, &camera, renderCollBox, collidedObject_callback);
    //NUEVO
    if (!isExtinguisherCollided || !pickObject)
        detectColls(&pickModels[0].collbox, pickModels[0].name, &camera, renderCollBox, collidedObject_callback);
    if (!isBoxCollided || !pickObject)
        detectColls(&pickModels[2].collbox, pickModels[2].name, &camera, renderCollBox, collidedObject_callback);

    detectColls(&physicsEnviroment, collidedPhysicsObject_callback);

    //NUEVO
    if (renderCountPhysicsCollided > 0)
    {
        renderCountPhysicsCollided -= 0.1;
        showSecondMessage = true;
        Text->RenderText(textRendererPhysicsObjectsCollided, -0.45f, 0.7f, 0.001f, glm::vec3(1.0f, 1.0f, 0.0f));
        secondMessage->Draw(glm::vec2(0.0f, -0.75f), 1.0f);
    }
}
void physicsUpdate(Shader *shader)
{
    int div = 3;
    const rbVec3 G(0.0f, rbReal(-9.8), 0.0f); //FUERZA DE GRAVEDAD 9.8
    for (int i = 0; i < rigidModels.size(); i++)
    {
        //DEFINIMOS LA FUERZA Y DIBUJAMOS EL OBJETO
        rigidModels[i].getRigidbox()->SetForce(G);
        rigidModels[i].Draw(*shader);
    }
    physicsEnviroment.Update(deltaTime, div);
}
void drawModels(Shader *shader, glm::mat4 view, glm::mat4 projection)
{
    //DEFINIMOS EL BRILLO  DEL MATERIAL
    shader->setFloat("material.shininess", 60.0f);
    setMultipleLight(shader, pointLightPositions);
    //cruce.Draw(*shader);
    //for (int i = 0; i < models.size(); i++)
    //{
    //    //SI SE RECOGIO EL OBJETO
    //    shader->use();
    //    models[i].Draw(*shader);
    //    shader->notUse();
    //    detectColls(&models[i].collbox, models[i].name, &camera, renderCollBox, collidedObject_callback);
    //}

    modelsIlumination[0].Draw(*shader);
    modelsIlumination[1].Draw(*shader);
    modelsIlumination[2].Draw(*shader);

    modelsIlumination[3].Draw(*shader);
    modelsIlumination[4].Draw(*shader);
    modelsIlumination[5].Draw(*shader);

}
void loadAnimations(glm::mat4 view, glm::mat4 projection)
{
    //AGUA ANIMACIÓN
    if (watterAnimY <= 4.0f && watterAnimX <= 4.0f && !watterOut)
    {
        watterAnimX += 0.001f;
        watterAnimY += 0.001f;
    }
    else
    {
        watterOut = true;
        if (watterAnimY > 0.0f && watterAnimX > 0.0f && watterOut)
        {
            watterAnimX -= 0.001f;
            watterAnimY -= 0.001f;
        }
        else
            watterOut = false;
    }

    plane.draw(watterAnimX, watterAnimY, view, projection);

    //:::: ANIMACIONES 3D:::://
    glm::vec3 doorLeftPos = models[3].getPosition();
    glm::vec3 doorRightPos = models[4].getPosition();
    if (doorAnim)
    {
        if (openDoor)
        {

            if (doorLeftPos.z < 17.68)
                doorLeftPos.z += 0.1;
            if (doorRightPos.z > 11.99)
                doorRightPos.z -= 0.1;
            else
            {
                openDoor = false;
                waitingDoor = 30;
            }
        }
        else
        {
            if (waitingDoor >= 0.0)
                waitingDoor -= 0.1;
            else
            {

                if (doorLeftPos.z > 15.68)

                    doorLeftPos.z -= 0.1;
                if (doorRightPos.z < 13.99)
                    doorRightPos.z += 0.1;
                else
                    doorAnim = false;
            }
        }
        models[3].setPosition(doorLeftPos);
        models[4].setPosition(doorRightPos);
    }

    //:::: ANIMACIÓN DE BILLBOARDS:::://
    if (changeSprite)
    {
        if (spriteX <= 4.0f)
        {

            spriteX += 0.2f;
        }
        else
        {
            spriteX = 0.0f;
        }
        qt.changeSprite(round(spriteX), spriteY);
        qt2.changeSprite(round(spriteX), spriteY);
        qt3.changeSprite(round(spriteX), spriteY);
        //HUD
        qt.Draw(glm::vec2(-0.8f, 0.8f), 0.2);
        qt2.Draw(glm::vec2(-0.6f, 0.8f), 0.2);
        qt3.Draw(glm::vec2(-0.4f, 0.8f), 0.2);
    }

    //Animación de fuego
    if (fireAnimY <= 2.0f)
    {

        if (fireAnimX <= 4.0f)
        {

            fireAnimX += 0.3f;
        }
        else
        {
            fireAnimX = 0.0f;
            fireAnimY += 1.0f;
        }
    }
    else
    {
        fireAnimX = 0.0f;
        fireAnimY = 0.0f;
    }

    //Animación de reloj
    if (clockAnimY <= 2.0f)
    {

        if (clockAnimX <= 4.0f)
        {

            clockAnimX += 0.3f;
        }
        else
        {
            clockAnimX = 0.0f;
            clockAnimY += 1.0f;
        }
    }
    else
    {
        clockAnimX += 0.0f;
        clockAnimY = 0.0f;
    }

    //HUMO
    if (humoAnimY <= 2.0f)
    {

        if (humoAnimX <= 4.0f)
        {

            humoAnimX += 0.3f;
        }
        else
        {
            humoAnimX = 0.0f;
            humoAnimY += 1.0f;
        }
    }
    else
    {
        humoAnimX += 0.0f;
        humoAnimY = 0.0f;
    }

    //Animaciones de Cafe
    if (cafeAnimY <= 2.0f)
    {

        if (cafeAnimX <= 4.0f)
        {

            cafeAnimX += 0.3f;
        }
        else
        {
            cafeAnimX = 0.0f;
            cafeAnimY += 1.0f;
        }
    }
    else
    {
        cafeAnimX += 0.0f;
        cafeAnimY = 0.0f;
    }

    //Animacion de Explosion
    if (exploAnimY <= 2.0f)
    {

        if (exploAnimX <= 4.0f)
        {

            exploAnimX += 0.3f;
        }
        else
        {
            exploAnimX = 0.0f;
            exploAnimY += 1.0f;
        }
    }
    else
    {
        exploAnimX += 0.0f;
        exploAnimY = 0.0f;
    }

    //Animacion reloj (el mio)
    if (relojAnimY <= 2.0f)
    {

        if (relojAnimX <= 4.0f)
        {

            relojAnimX += 0.3f;
        }
        else
        {
            relojAnimX = 0.0f;
            relojAnimY += 1.0f;
        }
    }
    else
    {
        relojAnimX += 0.0f;
        relojAnimY = 0.0f;
    }

    fireAnimBillboard.Draw(camera, round(fireAnimX), round(fireAnimY));
    clockAnimBillboard.Draw(camera, round(clockAnimX), round(clockAnimY));
    humoAnimBillboard.Draw(camera, round(humoAnimX), round(humoAnimY));
    relojAnimBillboard.Draw(camera, round(relojAnimX), round(relojAnimY));
    explosionAnimBillboard.Draw(camera, round(exploAnimX), round(exploAnimY));
    cafeAnimBillboard.Draw(camera, round(cafeAnimX), round(cafeAnimY));
    //:::: GUI:::://
    carIcon.Draw(glm::vec2(0.8f, 0.8f), 0.2);

    if (showMainMessage)
    {
        p2.Draw(glm::vec2(-0.22f, 0.05f), 0.2);
        p1.Draw(glm::vec2(-0.02f, 0.05f), 0.2);
        p3.Draw(glm::vec2(0.18f, 0.05f), 0.2);
        mainMessage.Draw(glm::vec2(0.0f, 0.0f), 1.0f);
    }

    //:::: PARTICULAS:::://
    particles.OnUpdate(0.01f);
    particles.Draw(glm::vec3(-5.5f, -3.5f, -2.5f), view, projection);

    if (respawnCount >= 0.5)
    {
        for (int i = 0; i < 25; i++)
            particles.Emit(m_Particle);
        updateParticles = false;
        respawnCount = 0;
    }
}
void setSimpleLight(Shader *shader)
{
    shader->setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
    shader->setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
    shader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);
    shader->setInt("lightType", (int)lightType);
    shader->setVec3("light.position", lightPos);
    shader->setVec3("light.direction", lightDir);
    shader->setFloat("light.cutOff", glm::cos(glm::radians(12.5f)));
    shader->setFloat("light.outerCutOff", glm::cos(glm::radians(17.5f)));
    shader->setVec3("viewPos", camera.Position);
    shader->setFloat("light.constant", 0.2f);
    shader->setFloat("light.linear", 0.02f);
    shader->setFloat("light.quadratic", 0.032f);
    shader->setFloat("material.shininess", 60.0f);
}
void setMultipleLight(Shader *shader, vector<glm::vec3> pointLightPositions)
{
    shader->setVec3("viewPos", camera.Position);

    //ejercicio

    shader->setVec3("dl.direction", pointLightPositions[0]);
    shader->setVec3("dl.ambient", mainLight.x, mainLight.y, mainLight.z);
    shader->setVec3("dl.diffuse", mainLight.x, mainLight.y, mainLight.z);
    shader->setVec3("dl.specular", mainLight.x, mainLight.y, mainLight.z);

    //luz roja
    shader->setVec3("plred.position", ejercicioIlumPositions[0]);
    shader->setVec3("plred.ambient", 1.0f, 0.0f, 0.0f);
    shader->setVec3("plred.diffuse", 1.0f, 0.0f, 0.0f);
    shader->setVec3("plred.specular", 1.0f, 0.0f, 0.0f);
    shader->setFloat("plred.constant", 1.0f);
    shader->setFloat("plred.linear", 0.09);
    shader->setFloat("plred.quadratic", 0.032);

    //luz verde
    shader->setVec3("plgreen.position", ejercicioIlumPositions[1]);
    shader->setVec3("plgreen.ambient", 0.0f, 1.0f, 0.0f);
    shader->setVec3("plgreen.diffuse", 0.0f, 1.0f, 0.0f);
    shader->setVec3("plgreen.specular", 0.0f, 1.0f, 0.0f);
    shader->setFloat("plgreen.constant", 1.0f);
    shader->setFloat("plgreen.linear", 0.09);
    shader->setFloat("plgreen.quadratic", 0.032);

    //luz azul
    shader->setVec3("plblue.position", ejercicioIlumPositions[2]);
    shader->setVec3("plblue.ambient", 0.0f, 0.0f, 1.0f);
    shader->setVec3("plblue.diffuse", 0.0f, 0.0f, 1.0f);
    shader->setVec3("plblue.specular", 0.0f, 0.0f, 1.0f);
    shader->setFloat("plblue.constant", 1.0f);
    shader->setFloat("plblue.linear", 0.09);
    shader->setFloat("plblue.quadratic", 0.032);

    //luz roja
    shader->setVec3("plred.position", ejercicioIlumPositions[3]);
    shader->setVec3("plred.ambient", 1.0f, 0.0f, 0.0f);
    shader->setVec3("plred.diffuse", 1.0f, 0.0f, 0.0f);
    shader->setVec3("plred.specular", 1.0f, 0.0f, 0.0f);
    shader->setFloat("plred.constant", 1.0f);
    shader->setFloat("plred.linear", 0.09);
    shader->setFloat("plred.quadratic", 0.032);

    //luz verde
    shader->setVec3("plgreen.position", ejercicioIlumPositions[4]);
    shader->setVec3("plgreen.ambient", 0.0f, 1.0f, 0.0f);
    shader->setVec3("plgreen.diffuse", 0.0f, 1.0f, 0.0f);
    shader->setVec3("plgreen.specular", 0.0f, 1.0f, 0.0f);
    shader->setFloat("plgreen.constant", 1.0f);
    shader->setFloat("plgreen.linear", 0.09);
    shader->setFloat("plgreen.quadratic", 0.032);

    //luz azul
    shader->setVec3("plblue.position", ejercicioIlumPositions[5]);
    shader->setVec3("plblue.ambient", 0.0f, 0.0f, 1.0f);
    shader->setVec3("plblue.diffuse", 0.0f, 0.0f, 1.0f);
    shader->setVec3("plblue.specular", 0.0f, 0.0f, 1.0f);
    shader->setFloat("plblue.constant", 1.0f);
    shader->setFloat("plblue.linear", 0.09);
    shader->setFloat("plblue.quadratic", 0.032);

    //spotlight
    shader->setVec3("sl.position", ejercicioIlumPositions[6]);
    shader->setVec3("sl.direction", glm::vec3(0.0f, -1.0f, 0.0f));
    shader->setVec3("sl.ambient", 0.0f, 0.0f, 0.0f);
    shader->setVec3("sl.diffuse", 1.0f, 1.0f, 1.0f);
    shader->setVec3("sl.specular", 1.0f, 1.0f, 1.0f);
    shader->setFloat("sl.constant", 1.0f);
    shader->setFloat("sl.linear", 0.004);
    shader->setFloat("sl.quadratic", 0.002);
    shader->setFloat("sl.cutOff", glm::cos(glm::radians(12.5f)));
    shader->setFloat("sl.outerCutOff", glm::cos(glm::radians(18.0f)));

    /*shader->setVec3("sl.position", ejercicioIlumPositions[7]);
    shader->setVec3("sl.direction", glm::vec3(0.0f, -1.0f, 0.0f));
    shader->setVec3("sl.ambient", 0.0f, 0.0f, 0.0f);
    shader->setVec3("sl.diffuse", 1.0f, 1.0f, 1.0f);
    shader->setVec3("sl.specular", 1.0f, 1.0f, 1.0f);
    shader->setFloat("sl.constant", 1.0f);
    shader->setFloat("sl.linear", 0.09);
    shader->setFloat("sl.quadratic", 0.032);
    shader->setFloat("sl.cutOff", glm::cos(glm::radians(12.5f)));
    shader->setFloat("sl.outerCutOff", glm::cos(glm::radians(18.0f)));

    shader->setVec3("sl.position", ejercicioIlumPositions[8]);
    shader->setVec3("sl.direction", glm::vec3(0.0f, -1.0f, 0.0f));
    shader->setVec3("sl.ambient", 0.0f, 0.0f, 0.0f);
    shader->setVec3("sl.diffuse", 1.0f, 1.0f, 1.0f);
    shader->setVec3("sl.specular", 1.0f, 1.0f, 1.0f);
    shader->setFloat("sl.constant", 1.0f);
    shader->setFloat("sl.linear", 0.09);
    shader->setFloat("sl.quadratic", 0.032);
    shader->setFloat("sl.cutOff", glm::cos(glm::radians(12.5f)));
    shader->setFloat("sl.outerCutOff", glm::cos(glm::radians(18.0f)));*/

    //switch de iluminacion
    shader->setBool("ejercicioIluminacion", ejercicioIluminacion);

    //normal

    shader->setVec3("dirLights[0].direction", pointLightPositions[0]);
    shader->setVec3("dirLights[0].ambient", mainLight.x, mainLight.y, mainLight.z);
    shader->setVec3("dirLights[0].diffuse", mainLight.x, mainLight.y, mainLight.z);
    shader->setVec3("dirLights[0].specular", mainLight.x, mainLight.y, mainLight.z);

    shader->setVec3("dirLights[1].direction", pointLightPositions[1]);
    shader->setVec3("dirLights[1].ambient", 0.05f, 0.05f, 0.05f);
    shader->setVec3("dirLights[1].diffuse", 0.4f, 0.4f, 0.4f);
    shader->setVec3("dirLights[1].specular", 0.5f, 0.5f, 0.5f);

    shader->setVec3("dirLights[2].direction", pointLightPositions[2]);
    shader->setVec3("dirLights[2].ambient", 0.05f, 0.05f, 0.05f);
    shader->setVec3("dirLights[2].diffuse", 0.4f, 0.4f, 0.4f);
    shader->setVec3("dirLights[2].specular", 0.5f, 0.5f, 0.5f);

    shader->setVec3("dirLights[3].direction", pointLightPositions[3]);
    shader->setVec3("dirLights[3].ambient", 0.05f, 0.05f, 0.05f);
    shader->setVec3("dirLights[3].diffuse", 0.4f, 0.4f, 0.4f);
    shader->setVec3("dirLights[3].specular", 0.5f, 0.5f, 0.5f);

    shader->setVec3("pointLights[0].position", pointLightPositions[0]);
    shader->setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
    shader->setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
    shader->setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
    shader->setFloat("pointLights[0].constant", 1.0f);
    shader->setFloat("pointLights[0].linear", 0.09);
    shader->setFloat("pointLights[0].quadratic", 0.032);

    shader->setVec3("pointLights[1].position", pointLightPositions[1]);
    shader->setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
    shader->setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
    shader->setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
    shader->setFloat("pointLights[1].constant", 1.0f);
    shader->setFloat("pointLights[1].linear", 0.09);
    shader->setFloat("pointLights[1].quadratic", 0.032);

    shader->setVec3("pointLights[2].position", pointLightPositions[2]);
    shader->setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
    shader->setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
    shader->setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
    shader->setFloat("pointLights[2].constant", 1.0f);
    shader->setFloat("pointLights[2].linear", 0.09);
    shader->setFloat("pointLights[2].quadratic", 0.032);

    shader->setVec3("pointLights[3].position", pointLightPositions[3]);
    shader->setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
    shader->setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
    shader->setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
    shader->setFloat("pointLights[3].constant", 1.0f);
    shader->setFloat("pointLights[3].linear", 0.09);
    shader->setFloat("pointLights[3].quadratic", 0.032);

    shader->setVec3("spotLights[0].position", pointLightPositions[0]);
    shader->setVec3("spotLights[0].direction", glm::vec3(0.2, 0.8, 0.2));
    shader->setVec3("spotLights[0].ambient", 0.0f, 0.0f, 0.0f);
    shader->setVec3("spotLights[0].diffuse", 1.0f, 1.0f, 1.0f);
    shader->setVec3("spotLights[0].specular", 1.0f, 1.0f, 1.0f);
    shader->setFloat("spotLights[0].constant", 1.0f);
    shader->setFloat("spotLights[0].linear", 0.09);
    shader->setFloat("spotLights[0].quadratic", 0.032);
    shader->setFloat("spotLights[0].cutOff", glm::cos(glm::radians(12.5f)));
    shader->setFloat("spotLights[0].outerCutOff", glm::cos(glm::radians(15.0f)));

    // spotLight
    shader->setVec3("spotLights[1].position", pointLightPositions[1]);
    shader->setVec3("spotLights[1].direction", camera.Front);
    shader->setVec3("spotLights[1].ambient", 0.0f, 0.0f, 0.0f);
    shader->setVec3("spotLights[1].diffuse", 1.0f, 1.0f, 1.0f);
    shader->setVec3("spotLights[1].specular", 1.0f, 1.0f, 1.0f);
    shader->setFloat("spotLights[1].constant", 1.0f);
    shader->setFloat("spotLights[1].linear", 0.09);
    shader->setFloat("spotLights[1].quadratic", 0.032);
    shader->setFloat("spotLights[1].cutOff", glm::cos(glm::radians(12.5f)));
    shader->setFloat("spotLights[1].outerCutOff", glm::cos(glm::radians(15.0f)));

    shader->setVec3("spotLights[2].position", pointLightPositions[2]);
    shader->setVec3("spotLights[2].direction", camera.Front);
    shader->setVec3("spotLights[2].ambient", 0.0f, 0.0f, 0.0f);
    shader->setVec3("spotLights[2].diffuse", 1.0f, 1.0f, 1.0f);
    shader->setVec3("spotLights[2].specular", 1.0f, 1.0f, 1.0f);
    shader->setFloat("spotLights[2].constant", 1.0f);
    shader->setFloat("spotLights[2].linear", 0.09);
    shader->setFloat("spotLights[2].quadratic", 0.032);
    shader->setFloat("spotLights[2].cutOff", glm::cos(glm::radians(12.5f)));
    shader->setFloat("spotLights[2].outerCutOff", glm::cos(glm::radians(15.0f)));

    shader->setVec3("spotLights[3].position", pointLightPositions[3]);
    shader->setVec3("spotLights[3].direction", camera.Front);
    shader->setVec3("spotLights[3].ambient", 0.0f, 0.0f, 0.0f);
    shader->setVec3("spotLights[3].diffuse", 1.0f, 1.0f, 1.0f);
    shader->setVec3("spotLights[3].specular", 1.0f, 1.0f, 1.0f);
    shader->setFloat("spotLights[3].constant", 1.0f);
    shader->setFloat("spotLights[3].linear", 0.09);
    shader->setFloat("spotLights[3].quadratic", 0.032);
    shader->setFloat("spotLights[3].cutOff", glm::cos(glm::radians(12.5f)));
    shader->setFloat("spotLights[3].outerCutOff", glm::cos(glm::radians(15.0f)));

    shader->setInt("lightType", (int)lightType);
    shader->setInt("maxRenderLights", 1);
}
bool fullFillObjectives()
{

    bool fullFill = false;
    if (objectives.getValue("recogerPedido") &&
        objectives.getValue("dejarPedido") &&
        objectives.getValue("lataspared") &&
        objectives.getValue("chusaLatas") &&
        objectives.getValue("apagarFuego"))
    {
        fullFill = true;
        std::cout << "¡¡¡HAAAAAAS GANADOOOOOOOO!!" << std::endl;
    }
    else
    {
        system("cls");
        if (objectives.getValue("recogerPedido"))
        {
            std::cout << "Se ha recogido el pedido" << std::endl;
            //HACER COSAS AQUÍ
        }
        if (objectives.getValue("dejarPedido"))
        {
            std::cout << "Se ha dejado el pedido" << std::endl;
            //HACER COSAS AQUÍ
        }
        if (objectives.getValue("lataspared"))
        {
            std::cout << "Se ha tirado la basura" << std::endl;
            //HACER COSAS AQUÍ
        }
        if (objectives.getValue("chusaLatas"))
        {
            std::cout << "Se ha hecho chusa con las latas" << std::endl;
            //HACER COSAS AQUÍ
        }
        if (objectives.getValue("apagarFuego"))
        {
            std::cout << "Se ha apagado el fuego" << std::endl;
            //HACER COSAS AQUÍ
        }
    }
    return fullFill;
}
bool allCanCollided()
{
    return cansCollided.getValue("mundet") &&
           cansCollided.getValue("pepsi") &&
           cansCollided.getValue("coca") &&
           cansCollided.getValue("sprite");
}