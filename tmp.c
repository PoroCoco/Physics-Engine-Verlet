//mettre une particule d'air au millieu d'un banc de sable = remontée en diagonale gauche ? 
//update eau fausse ? eau droite gauche infini
//fixer bois qui se décale quand vapeur monte et sable/eau descend a coté.(Impression que cela arrive que sur le coté gauche pas le droit??) --> c'est bon :)

//revoir fonction ApplyVect pour faire y et x en même temps pas en boucle séparer car peut entrainer décalage de position donc donner mauvaise vélocité a mauvaise particule.

//chute libre particules pas bonne
//eau remonte descente sable et se retrouve dans endroits impossible....


//besoin d'avoir chute libre de sable et eau lorsque sol dessous enlevé. Pour avoir cela la regle "peux aller en bas a gauche/droite" doit avoir une condition supplémentaire sans trop la limiter.


//particule explorer avec particule cible, utilisation pathfinding A* pour aller à cible.
//besoin de faire une barre d'outils car trop de touches utilisées.

//gcc -std=c99 -Wall src/main.c -o bin/prog -I include -L lib -lmingw32 -lSDL2main -lSDL2

//particule.h->pathfinding.h
#include <SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include "config.h"
#include "pathfinding.c"

// SDL_RENDERER_SOFTWARE; SDL_RENDERER_ACCELERATED; SDL_RENDERER_PRESENTVSYNC






void fillArrayWithMat(particule_t world[LIGNE][COLONNE], uint mat_id);


void setParticuleXY(particule_t world[LIGNE][COLONNE], uint mat_id, uint x, uint y);
void setParticuleXY_10by10(particule_t world[LIGNE][COLONNE], uint mat_id, uint x, uint y);

void updateArray(particule_t world[LIGNE][COLONNE]);

//prototype des fonctions contenant les règles de chaque particules.
void updateSand(particule_t world[LIGNE][COLONNE], uint x, uint y);

void updateWater(particule_t world[LIGNE][COLONNE], uint x, uint y);

void updateBois(particule_t world[LIGNE][COLONNE], uint x, uint y);

void updateFeu(particule_t world[LIGNE][COLONNE], uint x, uint y);

void updateAir(particule_t world[LIGNE][COLONNE], uint x, uint y);

void updateVapeur(particule_t world[LIGNE][COLONNE], uint x, uint y);

void updatePlayer(particule_t world[LIGNE][COLONNE], uint x, uint y);




void setArrayToUpdate(particule_t world[LIGNE][COLONNE]);

void randomizeWorld(particule_t world[LIGNE][COLONNE]);

void applyVect(int x, int y,particule_t world[LIGNE][COLONNE], int id);

bool catchedOnFire(int flammability);

void burnAround(particule_t world[LIGNE][COLONNE],int y, int x);

void looseLife(particule_t world[LIGNE][COLONNE],int y, int x, int life_lost, int chance);

void displayParticuleInfos(particule_t world[LIGNE][COLONNE], uint x, uint y);

void highlightParticule(particule_t world[LIGNE][COLONNE], uint x, uint y, int R, int G, int B);

bool isSurrounedAroundBy(particule_t world[LIGNE][COLONNE], uint x, uint y, int mat_id);

bool isSurrounedSideBy(particule_t world[LIGNE][COLONNE], uint x, uint y, int mat_id);

void burningColor(particule_t world[LIGNE][COLONNE], int y, int x,int mat_id);

bool isNotOnAFallTrajectory(particule_t world[LIGNE][COLONNE], uint x, uint y);

void setGravity(int valeur);
void displayTargetPos(particule_t world[LIGNE][COLONNE]);

bool deletePlayerParticule(particule_t world[LIGNE][COLONNE]);


int playerMovement;

int main(int argc, char ** argv){


    vector_t first_point; 
    vector_t second_point; 
    bool recording_point_second =false;
    bool recording_point = false;
    bool b;
    
    static particule_t world[LIGNE][COLONNE];

    srand(8575); //initialise le random avec une seed
    uint elementSelected = 0;
    setGravity(def_gravity);

    

    //Execution programme

    

    SDL_bool program_launched = SDL_TRUE;
    //fillArrayWithMat(world,0);

    while(program_launched)
    {
        playerMovement = 4;
        uint start_time = SDL_GetPerformanceCounter();
        bool button_mousedown;
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_MOUSEBUTTONDOWN:
                    button_mousedown = true;
                    break;

                case SDL_MOUSEBUTTONUP:
                    button_mousedown = false;
                    break;


                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym)
                    {
                    case SDLK_ESCAPE:
                        printf("Touche echappe appuiee, fermeture du programme\n");
                        program_launched = SDL_FALSE;
                        continue;
                    
                    case SDLK_a:
                        printf("appui a\n");
                        fillArrayWithMat(world,0);
                        continue;

                    case SDLK_z:
                        printf("appui z\n");
                        fillArrayWithMat(world,1);
                        continue;

                    case SDLK_w:
                        printf("appui w : élément mis = air\n");
                        elementSelected = 0;
                        continue;
                    
                    case SDLK_x:
                        printf("appui x : élément mis = sable\n");
                        elementSelected = 1;
                        continue;
                    
                    case SDLK_c:
                        printf("appui c : élément mis = eau\n");
                        elementSelected = 2;
                        continue;

                    case SDLK_r:
                        printf("appui r : Randomization du monde\n");
                        randomizeWorld(world);
                        continue;

                    case SDLK_v:
                        printf("appui v : élément mis = bois\n");
                        elementSelected = 3;
                        continue;

                    case SDLK_b:
                        printf("appui b : élément mis = feu\n");
                        elementSelected = mat_id_feu;
                        continue;

                    case SDLK_n:
                        printf("appui n : élément mis = vapeur\n");
                        elementSelected = mat_id_vapeur;
                        continue;

                    case SDLK_m:
                        printf("appui n : élément mis = EXPLORER\n");
                        elementSelected = mat_id_explorer;
                        continue;

                    case SDLK_l:
                        updateArray(world);
                        continue;

                    case SDLK_i:
                        printf("info mod selectionné :\n");
                        elementSelected = -1;
                        continue;

                    case SDLK_p:
                        printf("Paint mod selectionné :\n");
                        elementSelected = -2;
                        continue;
                    
                    case SDLK_g:
                        printf("Changement gravité\n");
                        setGravity(-gravity);
                        continue;

                    case SDLK_t:
                        printf("appui t : élément mis = PLAYER\n");
                        elementSelected = mat_id_player;
                        continue;

                    case SDLK_y:
                        elementSelected = -3;
                        continue;

                    case SDLK_u:
                        elementSelected = -4;
                        GOTOPOS = 1;
                        continue;

                    case SDLK_h:
                        printf("Choissisez les extremites du labyrinthe\n");
                        elementSelected = -3;
                        continue;
                    
                    case SDLK_DOWN:
                        playerMovement = 0;
                        continue;

                    case SDLK_LEFT:
                        playerMovement = 1;
                        continue;
                    
                    case SDLK_UP:
                        playerMovement = 2;
                        continue;

                    case SDLK_RIGHT:
                        playerMovement = 3;
                        continue;

                    default:
                        continue;
                    }

                case SDL_KEYUP:
                    switch (event.key.keysym.sym)
                    {
                    case SDLK_a:
                        printf("relachement de a\n");
                        continue;
        
                    default:
                        continue;
                    }

                case SDL_QUIT:
                    program_launched = SDL_FALSE;
                    break;

                default:
                    break;
            }
        }
        if(button_mousedown){
            //printf("Clique en %d / %d\n",event.button.x/PIXEL_WIDTH, event.button.y/PIXEL_HEIGHT);
            if(elementSelected == -1){
                displayParticuleInfos(world,event.button.x/PIXEL_WIDTH,event.button.y/PIXEL_HEIGHT);
            }else if(elementSelected == -2){
                highlightParticule(world, event.button.x/PIXEL_WIDTH, event.button.y/PIXEL_HEIGHT,TARGET_R,TARGET_G,TARGET_B);
            }else if(elementSelected == -3){
                if(recording_point == false){
                    first_point.x = event.button.x/PIXEL_WIDTH;
                    first_point.y = event.button.y/PIXEL_HEIGHT;
                    recording_point = true;
                }
                if(recording_point_second){
                    second_point.x = event.button.x/PIXEL_WIDTH;
                    second_point.y = event.button.y/PIXEL_HEIGHT;
                    recording_point = false;
                    recording_point_second = false;
                    if(first_point.x>= second_point.x || first_point.y >= second_point.y){
                        printf("Coordonnees donnees impossible\n");
                        second_point.x = 0;
                        second_point.y = 0;
                        first_point.x = 0;
                        first_point.y = 0;
                    }else{
                        b = generateMaze(world, first_point.x, first_point.y, second_point.x - first_point.x, second_point.y - first_point.y, mat_id_bois);
                        if(b){
                            printf("generated maze\n");
                        }else{
                            printf("maze couldn't generate\n");
                        }
                    }
                    
                }
            }else{
                setParticuleXY_10by10(world, elementSelected , event.button.x/PIXEL_WIDTH, event.button.y/PIXEL_HEIGHT);
            }
        }else if(elementSelected == -3 && recording_point){
            recording_point_second = true;
        }

        updateArray(world);
        renderArray(renderer,world,window);
        SDL_RenderPresent(renderer);
        setArrayToUpdate(world);
        uint end_time = SDL_GetPerformanceCounter();
        float freq_time = SDL_GetPerformanceFrequency();
        float elapsed = (end_time - start_time) / freq_time;
        float elapsedMS = (end_time - start_time) / freq_time*1000.0;
        float FPS_counter = 1.0/elapsed;
        float fps_delay = floor(16.666 - elapsedMS); 
        if(fps_delay>0){
            SDL_Delay(fps_delay);
            if(SHOW_FPS){
                printf("fps : %f\n",(1.0/(elapsedMS+fps_delay))*1000.0);
            }
        }else if (SHOW_FPS){
            printf("fps : %f\n",FPS_counter);
        }
    }






    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}

//gcc -std=c99 -Wall src/main.c -o bin/prog -I include -L lib -lmingw32 -lSDL2main -lSDL2



void fillArrayWithMat(particule_t world[LIGNE][COLONNE], uint mat_id){ //rempli le world avec 1 type de particule
    for(uint y = 0; y < LIGNE; y++){
        for(uint x = 0; x < COLONNE; x++){
            if (mat_id == mat_id_sable){
                world[y][x] = particule_sable;
            }else if(mat_id == mat_id_eau){
                world[y][x] = particule_eau;
            }else{
                world[y][x] = particule_air;
            }
        }
    }
    return;
}



void setParticuleXY(particule_t world[LIGNE][COLONNE], uint mat_id, uint x, uint y){ //place la particule correpondant au mat_id aux coordonnées XY
    if(isInWorldBoundaries(y,x)){
        if(mat_id == mat_id_sable){
            world[y][x] = particule_sable;
        }else if(mat_id == mat_id_air){
            world[y][x] = particule_air;
        }else if(mat_id == mat_id_eau){
            world[y][x] = particule_eau;
        }
    }
}

void updateArray(particule_t world[LIGNE][COLONNE]){ //parcours l'array soit de haut en bas, gauche à droite soit de haut en bas, de droite à gauche (permet de briser les patternes récurrents). Appelle la fonction d'update approprié pour chaque particules.
    int randomInt = rand() % 2;
    if(gravity_direction == 1){  //on update toujours notre monde depuis le bas qui dépend donc de la gravité positive ou négative.
        if(randomInt ==0){
            for(int y = LIGNE-1; y > -1; y--){
                for(int x = 0; x < COLONNE; x++){
                    if(world[y][x].id != mat_id_air && world[y][x].has_been_updated == false ){
                        if(world[y][x].id == mat_id_sable){
                            updateSand(world, x, y);
                        }else if(world[y][x].id == mat_id_eau){
                            updateWater(world,x,y);
                        }else if(world[y][x].id == mat_id_bois){
                            updateBois(world,x,y);
                        }else if(world[y][x].id == mat_id_feu){
                            updateFeu(world,x,y);
                        }else if(world[y][x].id == mat_id_vapeur){
                            updateVapeur(world,x,y);
                        }else if(world[y][x].id == mat_id_explorer){
                            updateExplorer(world,x,y);
                        }else if(world[y][x].id == mat_id_player){
                            updatePlayer(world,x,y);
                        }
                    }
                }
            }
        }else{
            for(int y = LIGNE-1; y > -1; y--){
                for(int x = COLONNE; x > -1 ; x = x-1){
                    if(world[y][x].id != mat_id_air && world[y][x].has_been_updated == false ){
                        if(world[y][x].id == mat_id_sable){
                            updateSand(world, x, y);
                        }else if(world[y][x].id == mat_id_eau){
                            updateWater(world, x, y);
                        }else if(world[y][x].id == mat_id_bois){
                            updateBois(world,x,y);
                        }else if(world[y][x].id == mat_id_feu){
                            updateFeu(world,x,y);
                        }else if(world[y][x].id == mat_id_vapeur){
                            updateVapeur(world,x,y);
                        }else if(world[y][x].id == mat_id_explorer){
                            updateExplorer(world,x,y);
                        }else if(world[y][x].id == mat_id_player){
                            updatePlayer(world,x,y);
                        }
                        
                        //}
                    }
                }
            }
        }
    }else{
        if(randomInt ==0){
            for(int y = 0; y < LIGNE; y++){
                for(int x = 0; x < COLONNE; x++){
                    if(world[y][x].id != mat_id_air && world[y][x].has_been_updated == false ){
                        if(world[y][x].id == mat_id_sable){
                            updateSand(world, x, y);
                        }else if(world[y][x].id == mat_id_eau){
                            updateWater(world,x,y);
                        }else if(world[y][x].id == mat_id_bois){
                            updateBois(world,x,y);
                        }else if(world[y][x].id == mat_id_feu){
                            updateFeu(world,x,y);
                        }else if(world[y][x].id == mat_id_vapeur){
                            updateVapeur(world,x,y);
                        }else if(world[y][x].id == mat_id_explorer){
                            updateExplorer(world,x,y);
                        }else if(world[y][x].id == mat_id_player){
                            updatePlayer(world,x,y);
                        }
                        //}
                    }
                }
            }
        }else{
            for(int y = 0; y < LIGNE; y++){
                for(int x = COLONNE; x > -1 ; x = x-1){
                    if(world[y][x].id != mat_id_air && world[y][x].has_been_updated == false ){
                        if(world[y][x].id == mat_id_sable){
                            updateSand(world, x, y);
                        }else if(world[y][x].id == mat_id_eau){
                            updateWater(world, x, y);
                        }else if(world[y][x].id == mat_id_bois){
                            updateBois(world,x,y);
                        }else if(world[y][x].id == mat_id_feu){
                            updateFeu(world,x,y);
                        }else if(world[y][x].id == mat_id_vapeur){
                            updateVapeur(world,x,y);
                        }else if(world[y][x].id == mat_id_explorer){
                            updateExplorer(world,x,y);
                        }else if(world[y][x].id == mat_id_player){
                            updatePlayer(world,x,y);
                        }
                        
                        //}
                    }
                }
            }
        }        
    }
}


void updateSand(particule_t world[LIGNE][COLONNE], uint x, uint y){
    world[y][x].has_been_updated = true;
    if(isInWorldBoundaries(y+gravity_direction,x) && (world[y+gravity_direction][x].id == mat_id_air || world[y+gravity_direction][x].id == mat_id_vapeur) ){ // si rien en dessous prend de la vitesse verticale.
        world[y][x].velocity.y += gravity;
        applyVect(x,y,world,1);
    }else if(isInWorldBoundaries(y+gravity_direction,x) && world[y+gravity_direction][x].id == mat_id_eau && world[y+gravity_direction][x].has_been_updated == false){ //si eau en dessous perd vitesse verticale(petit à petit) puis échange avec eau. (vitesse verticale inutile puisque pas d'appel à apply vect après)
        if(world[y][x].velocity.y > 0){
            world[y][x].velocity.y -= gravity;
        }
        if(rand()%100==1){
            particule_t tmp = world[y+gravity_direction][x];
            world[y+gravity_direction][x] = world[y][x];
            world[y][x] = tmp;
            world[y][x].has_been_updated = true;
            applyVect(x,y+gravity_direction,world,1);
        }
    }else{
        int randomInt = rand() % 2; //permet d'alterner entre toujours position gauche en premier ou toujours position droite en premier. 
        if(isSurrounedAroundBy(world,x,y,mat_id_eau)){
            randomInt = rand()%5;
        }
        if(randomInt==0){
            if(isInWorldBoundaries(y+gravity_direction,x-1) && (world[y+gravity_direction][x-1].id == mat_id_air || world[y+gravity_direction][x-1].id == mat_id_vapeur || world[y+gravity_direction][x-1].id == mat_id_eau) && isNotOnAFallTrajectory(world,x-1,y+gravity_direction)){ // si rien/gaz/eau en bas à gauche alors va-y.
                particule_t tmp = world[y+gravity_direction][x-1];
                world[y+gravity_direction][x-1] = world[y][x];
                world[y][x] = tmp;        
                applyVect(x-1,y+gravity_direction,world,1);
            }else if(isInWorldBoundaries(y+gravity_direction,x+1) && (world[y+gravity_direction][x+1].id == mat_id_air || world[y+gravity_direction][x+1].id == mat_id_vapeur|| world[y+gravity_direction][x+1].id == mat_id_eau)&& isNotOnAFallTrajectory(world,x+1,y+gravity_direction) ){ // si rien/gaz/eau en bas à droite alors va-y.
                particule_t tmp = world[y+gravity_direction][x+1];
                world[y+gravity_direction][x+1] = world[y][x];
                world[y][x] = tmp;  
                applyVect(x+1,y+gravity_direction,world,1);
            }else{
                world[y][x].velocity.y = 0;
            }
        }else if(randomInt == 1){
            if(isInWorldBoundaries(y+gravity_direction,x+1) && (world[y+gravity_direction][x+1].id == mat_id_air || world[y+gravity_direction][x+1].id == mat_id_vapeur || world[y+gravity_direction][x+1].id == mat_id_eau) && isNotOnAFallTrajectory(world,x+1,y+gravity_direction)){// si rien/gaz/eau en bas à droite alors va-y.
                particule_t tmp = world[y+gravity_direction][x+1];
                world[y+gravity_direction][x+1] = world[y][x];
                world[y][x] = tmp;
                applyVect(x+1,y+gravity_direction,world,1);
            }else if(isInWorldBoundaries(y+gravity_direction,x-1) && (world[y+gravity_direction][x-1].id == mat_id_air || world[y+gravity_direction][x-1].id == mat_id_vapeur || world[y+gravity_direction][x-1].id == mat_id_eau) && isNotOnAFallTrajectory(world,x-1,y+gravity_direction)){// si rien/gaz/eau en bas à gauche alors va-y.
                particule_t tmp = world[y+gravity_direction][x-1];
                world[y+gravity_direction][x-1] = world[y][x];
                world[y][x] = tmp; 
                applyVect(x-1,y+gravity_direction,world,1);
            }else{
                world[y][x].velocity.y = 0;
            }

        }    
    }
}

//chaque particule de l'array revient à l'état not updated.
void setArrayToUpdate(particule_t world[LIGNE][COLONNE]){ 
    for(uint y = 0; y < LIGNE; y++){
        for(uint x = 0; x < COLONNE; x++){
            world[y][x].has_been_updated = false;
        }
    }
}

void updateWater(particule_t world[LIGNE][COLONNE], uint x, uint y){
    world[y][x].has_been_updated = true;
    if(isInWorldBoundaries(y+gravity_direction,x) && world[y+gravity_direction][x].id == mat_id_bois && world[y+gravity_direction][x].life_time != lifetime_wood){ //éteint le bois qui est en train de bruler (check que les particules en dessous pour l'instant);
        world[y][x] = particule_vapeur;
        world[y+gravity_direction][x] = particule_bois;
        world[y+gravity_direction][x].life_time = lifetime_wood;
    }
    if(isInWorldBoundaries(y+gravity_direction,x) && (world[y+gravity_direction][x].id == mat_id_air || world[y+gravity_direction][x].id == mat_id_vapeur)){ //si rien/gaz en dessous alors prise de vitesse y et /2 la vitesse x
        world[y][x].velocity.y += gravity;
        world[y][x].velocity.x = world[y][x].velocity.x/2;
        applyVect(x,y,world,mat_id_eau);
    }else if(isSurrounedSideBy(world, x, y, mat_id_sable) && isInWorldBoundaries(y-gravity_direction,x)){ //si entouré sur les cotés par du sable alors échande de pos avec la particule de sable au dessus
        particule_t tmp = world[y-gravity_direction][x];
        world[y-gravity_direction][x] = world[y][x];
        world[y][x] = tmp;
    }else{
        
        if(isInWorldBoundaries(y+gravity_direction,x-1) && (world[y+gravity_direction][x-1].id == mat_id_air || world[y+gravity_direction][x-1].id == mat_id_vapeur) && isNotOnAFallTrajectory(world,x-1,y+gravity_direction) ) { //si possible va en bas à gauche 
            world[y][x].velocity.x = 0;
            world[y+gravity_direction][x-1] = world[y][x];
            world[y][x] = particule_air;  
            applyVect(x-1,y+gravity_direction,world,mat_id_eau);
        }else if(isInWorldBoundaries(y+gravity_direction,x+1) && (world[y+gravity_direction][x+1].id == mat_id_air || world[y+gravity_direction][x+1].id == mat_id_vapeur) && isNotOnAFallTrajectory(world,x+1,y+gravity_direction)){// si possible va en bas à droite
            world[y][x].velocity.x = 0;
            world[y+gravity_direction][x+1] = world[y][x];
            world[y][x] = particule_air;  
            applyVect(x+1,y+gravity_direction,world,mat_id_eau);
        }else if(isInWorldBoundaries(y,x-1) && (world[y][x-1].id == mat_id_air || world[y][x-1].id == mat_id_vapeur) && world[y][x].velocity.x <=0){ //si rien/gaz à gauche et pas de vitesse x positive alors gagne en vitesse x vers la gauche. 
            world[y][x].velocity.x += -2;
            applyVect(x,y,world,mat_id_eau); 
        }else if(isInWorldBoundaries(y,x+1) && (world[y][x+1].id == mat_id_air  || world[y][x+1].id == mat_id_vapeur)&& world[y][x].velocity.x >=0 ){//si rien/gaz à droite et pas de vitesse x positive alors gagne en vitesse x vers la droite. 
            world[y][x].velocity.x += +2; 
            applyVect(x,y,world,mat_id_eau);
        }else{
            world[y][x].velocity.x = 0;
            world[y][x].velocity.y = 0;
            world[y][x].has_been_updated = true;
        }
    }
}

//place les particules dans le monde (taille différente pour chaque, à améliorer)
void setParticuleXY_10by10(particule_t world[LIGNE][COLONNE], uint mat_id, uint x, uint y){ 
    if(isInWorldBoundaries(y,x)){
        if(mat_id == mat_id_sable){
            for(int i = -5; i<6;i = i +1){
                for(int j = -5; j<6; j = j+1){
                    if(isInWorldBoundaries(y+i,x+j)){
                        world[y+i][x+j] = particule_sable;
                    }
                }
            }
        }else if(mat_id == mat_id_air){
            for(int i = -3; i<4;i++){
                for(int j = -3; j<4;j++){
                    if(isInWorldBoundaries(y+i,x+j)){
                        world[y+i][x+j] = particule_air;
                    }
                }
            }
        }else if(mat_id == mat_id_eau){
            for(int i = -15; i<16;i++){
                for(int j = -15; j<16;j++){
                    if(isInWorldBoundaries(y+i,x+j)){
                        world[y+i][x+j] = particule_eau;
                    }
                }
            }
        }else if(mat_id == mat_id_bois){
            for(int i = -4; i<5;i++){
                for(int j = -4; j<5;j++){
                    if(isInWorldBoundaries(y+i,x+j)){
                        world[y+i][x+j] = particule_bois;
                    }
                }
            }
        }else if(mat_id == mat_id_feu){
            for(int i = -2; i<3;i++){
                for(int j = -2; j<3;j++){
                    if(isInWorldBoundaries(y+i,x+j)){
                        world[y+i][x+j] = particule_feu;
                    }
                }
            }
        }else if(mat_id == mat_id_vapeur){
            for(int i = -7; i<8;i++){
                for(int j = -7; j<8;j++){
                    if(isInWorldBoundaries(y+i,x+j)){
                        world[y+i][x+j] = particule_vapeur;
                    }
                }
            }
        }else if(mat_id == mat_id_explorer){
            world[y][x] = particule_explorer;
        }else if(mat_id == mat_id_player){
            deletePlayerParticule(world);
            world[y][x] = particule_player;
        }
    }
}


//chaque particules du monde devient une particule aléatoire, plus de chance pour que ce soit une particule d'air.
void randomizeWorld(particule_t world[LIGNE][COLONNE]){ 
    for(uint y = 0; y < LIGNE; y++){
        for(uint x = 0; x < COLONNE; x++){
            int random = rand()%8;
            if (random == mat_id_sable){
                world[y][x] = particule_sable;
            }else if(random == mat_id_eau){
                world[y][x] = particule_eau;
            }else if(random == mat_id_bois){
                world[y][x] = particule_bois;
            }else if(random == mat_id_feu){
                world[y][x] = particule_feu;
            }else if(random == mat_id_vapeur){
                world[y][x] = particule_vapeur;
            }else{
                world[y][x] = particule_air;
            }
        }
    }
}
void updateBois(particule_t world[LIGNE][COLONNE], uint x, uint y){ 
    world[y][x].has_been_updated = true;
    if(world[y][x].life_time != lifetime_wood){ //si tu n'as pas le lifetime normal alors c'est que tu brûles.
        looseLife(world, y, x, 3, 4);
        
        if(rand()%250 == 1 || (world[y][x].life_time<=100 && world[y][x].color.R == 139 && world[y][x].color.G == 69 && world[y][x].color.B == 19) ){         // 1/251 de chance de changer la couleur (change obligatoirement si aucun changement alors qu'il ne reste que moins 100 de lifetime)
            burningColor(world, y, x, mat_id_bois);
        }
        burnAround(world, y, x);
    }
    if(world[y][x].life_time <=0){ //Si lifetime <0 alors devient une particule de feu qui à déjà perdu le lifetime d'une particule de bois.
        world[y][x] = particule_feu;
        world[y][x].life_time = particule_feu.life_time - lifetime_wood;
    }

}

void updateFeu(particule_t world[LIGNE][COLONNE], uint x, uint y){
    world[y][x].has_been_updated = true;
    looseLife(world, y, x, 3, 4);
    if(rand()%15 == 1){         //1/16 chance de changer de couleur.
        burningColor(world, y, x, mat_id_feu);
    }
    if(world[y][x].life_time<=0){  
        world[y][x] = particule_air;
    }
    if(isInWorldBoundaries(y-gravity_direction,x) && world[y-gravity_direction][x].id == mat_id_eau){ // si tu eau en dessous alors particule de feu et d'eau deviennent des particules de vapeur
        world[y][x] = particule_vapeur; 
        world[y-gravity_direction][x] = particule_vapeur; 
        return;
    }else if(isInWorldBoundaries(y+gravity_direction,x) && world[y+gravity_direction][x].id == mat_id_eau){
        world[y+gravity_direction][x] = particule_air; 
        world[y][x] = particule_vapeur;
        if(isInWorldBoundaries(y+(2*gravity_direction),x) && world[y+(2*gravity_direction)][x].id == mat_id_air){
            world[y+(2*gravity_direction)][x] = particule_vapeur;
        }
        return;
    }else if(isInWorldBoundaries(y,x+1) && world[y][x+1].id == mat_id_eau){
        world[y][x+1] = particule_vapeur; 
        world[y][x] = particule_vapeur;
        return;
    }else if(isInWorldBoundaries(y,x-1) && world[y][x-1].id == mat_id_eau){
        world[y][x-1] = particule_air; 
        world[y][x] = particule_vapeur;
        return;
    }

    if(isInWorldBoundaries(y+gravity_direction,x) && (world[y+gravity_direction][x].id == mat_id_air || world[y+gravity_direction][x].id == mat_id_vapeur) ){ //si rien en dessous prend de la vitesse verticale.
        if(world[y][x].velocity.y == 0){
            world[y][x].velocity.y = gravity_direction;
        }else{
            world[y][x].velocity.y += (0.2* gravity_direction);
        }
        applyVect(x,y,world,mat_id_feu);
    }else{
    int randomInt = rand() % 2;
    world[y][x].velocity.y = 0;
        if(randomInt==0){
            if(isInWorldBoundaries(y+gravity_direction,x-1) && world[y+gravity_direction][x-1].id == mat_id_air){ // suit même regles que le sable pour physique.
                world[y+gravity_direction][x-1] = world[y][x];
                world[y][x] = particule_air;        
            }else if(isInWorldBoundaries(y+gravity_direction,x+1) && world[y+gravity_direction][x+1].id == mat_id_air){
                world[y+gravity_direction][x+1] = world[y][x];
                world[y][x] = particule_air;  
            }
        }else{
            if(isInWorldBoundaries(y+gravity_direction,x+1) && world[y+gravity_direction][x+1].id == mat_id_air){
                world[y+gravity_direction][x+1] = world[y][x];
                world[y][x] = particule_air;
            }else if(isInWorldBoundaries(y+gravity_direction,x-1) && world[y+gravity_direction][x-1].id == mat_id_air){
                world[y+gravity_direction][x-1] = world[y][x];
                world[y][x] = particule_air; 
            }

        } 
    }
    if(world[y][x].velocity.y == 0){
    burnAround(world, y, x);
    }
}

void applyVect(int x, int y,particule_t world[LIGNE][COLONNE], int id){
    int move_x = world[y][x].velocity.x;
    int move_y = world[y][x].velocity.y;
    int tmpPosOfY = y;
    if(world[y][x].velocity.x == 0 && world[y][x].velocity.y == 0 ){ //si aucune vitesse return
        return;
    }
    int direction;
    if(move_y>0){
        direction = 1; // si direction = 1 vers le bas
    }else{
        direction = -1; // si direction = -1 vers le haut
    }
    move_y = move_y*direction; //transforme en valeur absolue (positif : x/1=x, négatif : -x/-1 =x)

    if(id == mat_id_sable &&  move_y> sable_max_y_velocity){ //regarde si la particule ne dépasse pas sa vélocité maximale donnée. Si c'est le cas la vitesse remet au max pour ne pas dépasser.
        world[y][x].velocity.y = sable_max_y_velocity*direction;
    }
    else if(id == mat_id_feu &&  move_y> feu_max_y_velocity){
        world[y][x].velocity.y = feu_max_y_velocity*direction;
    }
    else if(id == mat_id_vapeur && move_y> vapeur_max_y_velocity){
        world[y][x].velocity.y = vapeur_max_y_velocity*direction;
    }
    else if(id == mat_id_eau && move_y > eau_max_y_velocity){
        world[y][x].velocity.y = eau_max_y_velocity*direction;
    }
    int newParticule_y = y;
    bool splashed = false;
    for(int i = 0; i<move_y;i++){       // on parcours chacune des possibles position y de la particule en fonction de sa vitesse verticale. on récupère la position de la dernière particule de rien(air)/gaz(vapeur) dans new_particule_y. 
        if(isInWorldBoundaries(y+(1*direction)+(i*direction),x)){
            if( (world[y+(1*direction)+(i*direction)][x].id == mat_id_air || world[y+(1*direction)+(i*direction)][x].id == mat_id_vapeur)){
                newParticule_y = y+(1*direction)+(i*direction);
                tmpPosOfY = y+(1*direction)+(i*direction);
            }else if(id == 1 && world[y+(1*direction)+(i*direction)][x].id == mat_id_eau ){ //si sable tombe dans eau profonde (au moins 3 particule d'eau de profondeur) alors splashed est true
                newParticule_y = y+(1*direction)+(i*direction);
                tmpPosOfY = y+(1*direction)+(i*direction);
                if(isInWorldBoundaries(y+(2*direction)+(i*direction),x) && world[y+(2*direction)+(i*direction)][x].id == mat_id_eau && isInWorldBoundaries(y+(3*direction)+(i*direction),x) && world[y+(3*direction)+(i*direction)][x].id == mat_id_eau ){
                    splashed = true;
                    break;
                }
            }else{
                break;
            }
        }
    }
    particule_t tmp = world[y][x];      //on échange la particule à laquelle on applique la vitesse avec la particule de position newParticule_y d'arrivé.
    world[y][x] = world[newParticule_y][x];
    world[newParticule_y][x] = tmp;
    if(splashed == true && isInWorldBoundaries(y,x)){ // si splashed alors les particule d'eau en dessous obtiennent la vélocité verticale (vers le haut) de la particule qui est tombé dessus. Puis on met une vélocité horizontale "aléatoire" pour donner effect d'éclaboussure.
        world[y][x].velocity.y = (move_y) *-1;
        if(rand()%2 == 0){
            if(isInWorldBoundaries(y,x-1) && world[y][x-1].id == mat_id_air){
                world[y][x].velocity.x -= 8;
            }else if(isInWorldBoundaries(y,x+1) && world[y][x+1].id == mat_id_air){
                world[y][x].velocity.x += 8;
            }
        }else{
            if(isInWorldBoundaries(y,x+1) && world[y][x+1].id == mat_id_air){
                world[y][x].velocity.x += 8;
            }else if(isInWorldBoundaries(y,x-1) && world[y][x-1].id == mat_id_air){
                world[y][x].velocity.x -= 8;
            }   
        }
    }

    //même chose avec la vitesse horizontale.
    if(move_x>0){
        direction = 1;
    }else{
        direction = -1;
    }
    move_x = move_x*direction; //transforme en valeur absolue (positif : x/1=x, négatif : -x/-1 =x)
    for(int i =0; i<move_x;i++){
        if(isInWorldBoundaries(tmpPosOfY,x+(1*direction)+(i*direction))){
            if(world[tmpPosOfY][x+(1*direction)+(i*direction)].id == mat_id_air || world[tmpPosOfY][x+(1*direction)+(i*direction)].id == mat_id_vapeur){
                particule_t tmp = world[tmpPosOfY][x+(1*direction)+(i*direction)];
                world[tmpPosOfY][x+(1*direction)+(i*direction)] = world[tmpPosOfY][x+(i*direction)];
                world[tmpPosOfY][x+(i*direction)] = tmp;
            }else{
                break;
            }
            if(id == mat_id_vapeur && i== 2){
            world[tmpPosOfY][x+(1*direction)+(i*direction)].velocity.x = 2*direction;
            break;
            }
            if(id == mat_id_eau && i== water_spread_speed){
                world[tmpPosOfY][x+(1*direction)+(i*direction)].velocity.x = water_spread_speed*direction;
                break;
            }
            if(i == 15){
                break;
            }
        } 
    }
}

//renvoi true si la particule donnée prend feu en fonction de la flammabilité définie dans particules.h
bool catchedOnFire(int mat_id){ 
    int flammability = 1;
    if(mat_id == mat_id_bois){
        flammability = bois_flammability;
    }
    return (rand()%flammability == 2);
}

//essais de brûler chaque particules autour des coordonnées y,x si dans limites du monde.
void burnAround(particule_t world[LIGNE][COLONNE],int y, int x){ 
    if(isInWorldBoundaries(y+1,x) && catchedOnFire(world[y+1][x].id)){
        world[y+1][x].life_time --;
    }if(isInWorldBoundaries(y+1,x-1) && catchedOnFire(world[y+1][x-1].id)){
        world[y+1][x-1].life_time --;
    }if(isInWorldBoundaries(y+1,x+1) && catchedOnFire(world[y+1][x+1].id)){
        world[y+1][x+1].life_time --;
    }if(isInWorldBoundaries(y,x-1) && catchedOnFire(world[y][x-1].id)){
        world[y][x-1].life_time --;
    }if(isInWorldBoundaries(y,x+1) && catchedOnFire(world[y][x+1].id)){
        world[y][x+1].life_time --;
    }if(isInWorldBoundaries(y-1,x-1) && catchedOnFire(world[y-1][x-1].id)){
        world[y-1][x-1].life_time --;
    }if(isInWorldBoundaries(y-1,x) && catchedOnFire(world[y-1][x].id)){
        world[y-1][x].life_time --;
    }if(isInWorldBoundaries(y-1,x+1) && catchedOnFire(world[y-1][x+1].id)){
        world[y-1][x+1].life_time --;
    }
}
// fais perdre "life_lost" vie à la particule avec 1/chance chance. 
void looseLife(particule_t world[LIGNE][COLONNE],int y, int x, int life_lost, int chance){ 
    if(rand()%chance == 1){
        world[y][x].life_time = world[y][x].life_time - life_lost; 
    }
}

void updateVapeur(particule_t world[LIGNE][COLONNE], uint x, uint y){
    world[y][x].has_been_updated = true;
    if(isInWorldBoundaries(y+gravity_direction,x)&&isInWorldBoundaries(y-gravity_direction,x) && world[y+gravity_direction][x].id == mat_id_air && world[y-gravity_direction][x].id == mat_id_vapeur){ // si vapeur au dessus et air en dessous alors perte de beaucoup de vie.(pour éviter les gros nuages)
        looseLife(world, y, x, 30, 5);
    }
    looseLife(world, y, x, 12, 20);
    if(world[y][x].life_time <=0){  //si vie à 0 devient particule d'air
        world[y][x] = particule_air;
        return;
    }
    if(isInWorldBoundaries(y-gravity_direction, x) && world[y-gravity_direction][x].id == mat_id_air){ //monte de 1 case par update si air au dessus.
        particule_t tmp = world[y-gravity_direction][x];
        world[y-gravity_direction][x] = world[y][x];
        world[y][x] = tmp;
    }else if(rand()%3==1){
        
        if(isInWorldBoundaries(y-gravity_direction,x-1) && world[y-gravity_direction][x-1].id == mat_id_air){ //si air en haut à gauche alors échange
            world[y][x].velocity.x = 0;
            particule_t tmp = world[y-gravity_direction][x-1];
            world[y-gravity_direction][x-1] = world[y][x];
            world[y][x] = tmp;
            applyVect(x,y,world,mat_id_vapeur);  
        }else if(isInWorldBoundaries(y-gravity_direction,x+1) && world[y-gravity_direction][x+1].id == mat_id_air){//si air en haut à droite alors échange
            world[y][x].velocity.x = 0;
            particule_t tmp = world[y-gravity_direction][x+1];
            world[y-gravity_direction][x+1] = world[y][x];
            world[y][x] = tmp;  
            applyVect(x,y,world,mat_id_vapeur);
        }else{
            if(isInWorldBoundaries(y,x-1) && world[y][x-1].id == mat_id_air && world[y][x].velocity.x <=0){ //sensé donner de la vitesse horizontale si aucune des autres règles ne peut etre exécutées mais ne semble pas fontionner.
                world[y][x].velocity.x += -1;
                applyVect(x,y,world,mat_id_vapeur); 
            }else if(isInWorldBoundaries(y,x+1) && world[y][x+1].id == mat_id_air && world[y][x].velocity.x >=0 ){
                world[y][x].velocity.x += +1; 
                applyVect(x,y,world,mat_id_vapeur);
            }else{
                world[y][x].velocity.x = 0;
            }
        }
    }
}

//print quelques infos sur la particule.
void displayParticuleInfos(particule_t world[LIGNE][COLONNE], uint x, uint y){ 
    if(isInWorldBoundaries(y,x)){
    printf("id = %d\n",world[y][x].id);
    printf("lifetime = %f\n",world[y][x].life_time);
    printf("velocity : x = %f, y = %f\n",world[y][x].velocity.x,world[y][x].velocity.y);
    printf("has been updated = %d\n",world[y][x].has_been_updated);
    }
}



//renvoie true si toutes les particules autour sont de type mat_id. (si mat_id = 1 et que true renvoyer alors la particule est entourée de sable)
bool isSurrounedAroundBy(particule_t world[LIGNE][COLONNE], uint x, uint y, int mat_id){ //renvoie true si toutes les particules autour sont de type mat_id. (si mat_id = 1 et que true renvoyer alors la particule est entourée de sable)
    bool surronded = true;
    if(isInWorldBoundaries(y-1,x-1) && world[y-1][x-1].id != mat_id){
        surronded = false;
    }if(isInWorldBoundaries(y-1,x) && world[y-1][x].id != mat_id){
        surronded = false;
    }if(isInWorldBoundaries(y-1,x+1) && world[y-1][x+1].id != mat_id){
        surronded = false;
    }if(isInWorldBoundaries(y,x-1) && world[y][x-1].id != mat_id){
        surronded = false;
    }if(isInWorldBoundaries(y,x+1) && world[y][x+1].id != mat_id){
        surronded = false;
    }if(isInWorldBoundaries(y+1,x-1) && world[y+1][x-1].id != mat_id){
        surronded = false;
    }if(isInWorldBoundaries(y+1,x) && world[y+1][x].id != mat_id){
        surronded = false;
    }if(isInWorldBoundaries(y+1,x+1) && world[y+1][x+1].id != mat_id){
        surronded = false;
    }
    return surronded;
}
//Similaire à isSurrounedAroundBy() mais ne regarde que droite,gauche,haut,bas.
bool isSurrounedSideBy(particule_t world[LIGNE][COLONNE], uint x, uint y, int mat_id){ 
    bool surronded = true;
    if(isInWorldBoundaries(y-1,x) && world[y-1][x].id != mat_id){
        surronded = false;
    }if(isInWorldBoundaries(y,x-1) && world[y][x-1].id != mat_id){
        surronded = false;
    }if(isInWorldBoundaries(y,x+1) && world[y][x+1].id != mat_id){
        surronded = false;
    }if(isInWorldBoundaries(y+1,x) && world[y+1][x].id != mat_id){
        surronded = false;
    }
    return surronded;
}

bool isNotOnAFallTrajectory(particule_t world[LIGNE][COLONNE], uint x, uint y){
    for(int i = 0; i<2;i++){ //regarde les 2 particules au dessus
        if(isInWorldBoundaries(y-i,x) && world[y-i][x].id == mat_id_eau){
            return true;
        }else if(isInWorldBoundaries(y-i,x) && world[y-i][x].id != mat_id_air && world[y-i][x].id != mat_id_vapeur){
            if(world[y-i][x].velocity.y + gravity >=i){
                return false;
            }else{
                return true;
            }
        }
    }
    return true;
}

//Selon la particule permet d'alterner les couleurs pour donner une impression de brûlure.
void burningColor(particule_t world[LIGNE][COLONNE], int y, int x,int mat_id){
    if(isInWorldBoundaries(y,x)){
        if(mat_id == mat_id_bois){
            int R,G,B;
            R = 220;
            G = 130;
            B = 51;
            int randR, randG, randB;
            randR = rand()%15;
            randG = rand()%27;
            randB = rand()%8;
            int minusOrPlusR,minusOrPlusG,minusOrPlusB;
            if(rand()%2 == 1){
                minusOrPlusR = 1;
            }else{
                minusOrPlusR = -1;
            }
            if(rand()%2 == 1){
                minusOrPlusG = 1;
            }else{
                minusOrPlusG = -1;
            }
            if(rand()%2 == 1){
                minusOrPlusB = 1;
            }else{
                minusOrPlusB = -1;
            }
            world[y][x].color.R = R + (randR*minusOrPlusR);
            world[y][x].color.G = G + (randG*minusOrPlusG);
            world[y][x].color.B = B + (randB*minusOrPlusB);
        }else if (mat_id == mat_id_feu){
            int R,G,B;
            R = 255;
            G = 92;
            B = 10;
            int randR, randG, randB;
            randR = rand()%18;
            randG = rand()%23;
            randB = rand()%10;
            int minusOrPlusR,minusOrPlusG,minusOrPlusB;
            if(rand()%2 == 1){
                minusOrPlusR = 0;
            }else{
                minusOrPlusR = -1;
            }
            if(rand()%2 == 1){
                minusOrPlusG = 1;
            }else{
                minusOrPlusG = -1;
            }
            if(rand()%2 == 1){
                minusOrPlusB = 1;
            }else{
                minusOrPlusB = -1;
            }
            world[y][x].color.R = R + (randR*minusOrPlusR);
            world[y][x].color.G = G + (randG*minusOrPlusG);
            world[y][x].color.B = B + (randB*minusOrPlusB);    
        }
    }
}

void setGravity(int valeur){
    gravity = valeur;
    if(gravity>0){
        gravity_direction = 1;
    }else if(gravity<0){
        gravity_direction = -1;
    }else{
        gravity_direction = 0;
    }
}


void displayTargetPos(particule_t world[LIGNE][COLONNE]){
    int x,y;
    getTargetPos(world,&x,&y);
    printf("x = %d, y = %d\n",x,y);
}

bool deletePlayerParticule(particule_t world[LIGNE][COLONNE]){
    for(uint y = 0; y < LIGNE; y++){
        for(uint x = 0; x < COLONNE; x++){
            if(world[y][x].id == mat_id_player){
                world[y][x] = particule_air;
                return true;
            }
        }
    }
    return false;
}


void updatePlayer(particule_t world[LIGNE][COLONNE], uint x, uint y){
    world[y][x].has_been_updated = true;
    if(playerMovement == 0){
        if(isInWorldBoundaries(y+1,x) && world[y+1][x].id == mat_id_air){
            particule_t tmp = world[y][x];
            world[y][x] = world[y+1][x];
            world[y+1][x] = tmp;
        }
    }else if(playerMovement == 1){
        if(isInWorldBoundaries(y,x-1) && world[y][x-1].id == mat_id_air){
            particule_t tmp = world[y][x];
            world[y][x] = world[y][x-1];
            world[y][x-1] = tmp;
        }
    }else if(playerMovement == 2){
        if(isInWorldBoundaries(y-1,x) && world[y-1][x].id == mat_id_air){
            particule_t tmp = world[y][x];
            world[y][x] = world[y-1][x];
            world[y-1][x] = tmp;
        }  
    }else if(playerMovement == 3){
        if(isInWorldBoundaries(y,x+1) && world[y][x+1].id == mat_id_air){
            particule_t tmp = world[y][x];
            world[y][x] = world[y][x+1];
            world[y][x+1] = tmp;
        }    
    }
}