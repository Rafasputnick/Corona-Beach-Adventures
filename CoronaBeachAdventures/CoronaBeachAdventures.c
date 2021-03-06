/* -- Corona Beach Adventures --
* Projeto desenvolvido no Senac Santo Amaro para o
* Projeto Integrado(PI) de Jogos Digitais do curso
* de Ci�ncia da Computa��o.
*
*  -- Feito por --
* Daniel Bortoleti Melo
* Lucas da Silva dos Santos
* Rafael Nascimento Rodrigues
*/

#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#include "sprites.h"
#include "audio.h"
#include "mapa.h"
#include "personagem.h"
#include "colisao.h"
#include "mascara.h"

#define FPS 60.f

#define NOME_JOGO "Corona Beach Adventures"

#define JANELA_LARGURA 640
#define JANELA_ALTURA 480

#define COR_FUNDO al_map_rgb(188, 198, 255)

#define GRAVIDADE 0.085
#define VELOCIDADE_MAX_X 6
#define VELOCIDADE_MAX_Y 20 
#define ALTURA_MAX_PULO 15
#define DELTA_PULO 1.2
#define REBOTE_Y 0.3

enum TECLAS { ESQUERDA, DIREITA, CIMA, BAIXO };

int main() {
	ALLEGRO_DISPLAY* janela = NULL;
	ALLEGRO_EVENT_QUEUE* fila_eventos = NULL;
	ALLEGRO_TIMER* timer = NULL;

	bool rodando = true;
	bool desenhar = true;
	bool teclas[] = { false,false,false,false };
	bool toggleMascara = false;
	bool pegouMascara = false;
	

	float velocidadeGravidade = -1;
	float alturaPulo = 0;
	unsigned frames = 0;
	unsigned nivel = 0;
	char niveis[3][7] = {"nivel1", "nivel2", "nivel3"};

	// Inicia o allegro
	if (!al_init()) {
		printf("Falha ao iniciar.\n");
		return 1;
	}

	// Inicia addon que da suporte as extensoes de imagem
	if (!al_init_image_addon()) {
		printf("Falha ao inicia addon de imagem.\n");
		return 1;
	}

	// Inicia addon que da suporte as extensoes de audio
	if (!al_init_acodec_addon()) {
		printf("Falha ao inicializar o codec de audio.\n");
		return 0;
	}

	// Inicia os dados primitivos do allegro
	if (!al_init_primitives_addon()) {
		printf("Falha ao inicializar os primitivos.\n");
		return 0;
	}

	// Cria um timer
	timer = al_create_timer(1.0 / FPS);
	if (!timer) {
		printf("Falha ao criar timer.\n");
		return 1;
	}

	// Cria uma janela de 640px por 480px
	janela = al_create_display(JANELA_LARGURA, JANELA_ALTURA);
	if (!janela) {
		printf("Falha ao criar uma janela.\n");
		return 1;
	}
	al_set_window_title(janela, NOME_JOGO);


	// Cria a fila de eventos
	fila_eventos = al_create_event_queue();
	if (!fila_eventos) {
		printf("Falha ao criar a fila de eventos.\n");
		return 1;
	}

	al_init_font_addon();

	// Inicializa��o do add-on para uso de fontes True Type 
	if (!al_init_ttf_addon()) {
		printf("Falha ao inicializar add-on allegro_ttf ");
		return 1;
	}

	// Carregando o arquivo de fonte
	ALLEGRO_FONT* fonte = NULL;
	fonte = al_load_font("arial.ttf", 14, 0);
	if (!fonte) {
		al_destroy_display(janela);
		printf("Falha a carregar a fonte");
		return 1;
	}
	// Instala audio
	if (!al_install_audio()) {
		printf("Falha ao instalar o audio.\n");
		return 1;
	}

	// Instala teclado
	if (!al_install_keyboard()) {
		printf("Falha ao instalar o teclado.\n");
		return 1;
	}

	// Instala mouse
	if (!al_install_mouse()) {
		printf("Falha ao instalar o mouse.\n");
		return 1;
	}

	// Registra as fontes dos eventos
	al_register_event_source(fila_eventos, al_get_display_event_source(janela));
	al_register_event_source(fila_eventos, al_get_timer_event_source(timer));
	al_register_event_source(fila_eventos, al_get_keyboard_event_source());
	al_register_event_source(fila_eventos, al_get_mouse_event_source());


	// Desenha uma tela preta
	al_clear_to_color(COR_FUNDO);
	al_flip_display();

	// Inicia o timer
	al_start_timer(timer);

	// Cria o mixer (e torna ele o mixer padrao), e adciona 5 samples de audio nele
	if (!al_reserve_samples(5)) {
		printf("Falha ao reservar amostrar de audio");
		return 1;
	}

	// Carrega uma audio
	Musica musicaFundo;
	musicaFundo = carregar_audio("musica.ogg");

	// Define que o stream vai tocar no modo repeat
	al_set_audio_stream_playmode(musicaFundo.som, ALLEGRO_PLAYMODE_LOOP);

	// Carrega mapa
	Mapa* mapa = carregar_mapa(niveis[nivel]);

	// Carrega personagem
	// Carregar um sprite
	ALLEGRO_BITMAP* bmp_botoes = carregar_imagem("ui.bmp");
	ALLEGRO_BITMAP* gameover = carregar_imagem("gameover.bmp");
	ALLEGRO_BITMAP* vitoria_img = carregar_imagem("vitoria.bmp");
	ALLEGRO_BITMAP* vida = carregar_imagem("corona_beach.bmp");
	ALLEGRO_BITMAP* mascarahud = carregar_imagem("corona_beach.bmp");

	Sprite* gameover_sprite = criar_sprite(gameover, 0, 0, JANELA_LARGURA, JANELA_ALTURA, 0);
	Sprite* vitoria_sprite = criar_sprite(vitoria_img, 0, 0, JANELA_LARGURA, JANELA_ALTURA, 0);
	Sprite* botoes = criar_sprite(bmp_botoes, 0, 0, 16, 16, 0);
	Personagem* personagem = carrega_personagem(botoes, posicao_inicial.x, posicao_inicial.y, 16, 16);
	Sprite* vida_sprite = criar_sprite(vida, 0, 32, 16, 16, 0);
	Sprite* mascara_sprite = criar_sprite(mascarahud, 16, 32, 16, 16, 0);

	
	//cria um vetor que guarda a velocidade do personagem
	Vetor2D velocidadePersonagem;
	velocidadePersonagem.y = 0;
	velocidadePersonagem.x = 0;

	//cria a mascara para uso do personagem
	Mascara* mascara = carrega_mascara(100);  // mascara leve 100 - mascara media 200 - mascara pesada 300

	Vetor2D aux = { 0, 0 };
	Vetor2D aux2 = { 50, 0 };
	bool vitoria = false;
	bool proximo_level = false;
	char *texto = "use mascara";
	Vetor2D posicao_texto = { 500, 10 };

	// Loop principal do jogo
	while (rodando) {

		ALLEGRO_EVENT evento;

		// Busca o evento (se houver)
		al_wait_for_event(fila_eventos, &evento);

		// coloca a musica de fundo na lista
		al_attach_audio_stream_to_mixer(musicaFundo.som, al_get_default_mixer());

		if (evento.type) {

			// Verifica se o timer ta rodando e desenha
			if (evento.type == ALLEGRO_EVENT_TIMER) {
				desenhar = true;
				frames++;
			}

			// Verifica evento de clicar em uma tecla
			if (evento.type == ALLEGRO_EVENT_KEY_DOWN) {
				switch (evento.keyboard.keycode) {
				case ALLEGRO_KEY_LEFT:
					teclas[ESQUERDA] = true;
					break;

				case ALLEGRO_KEY_RIGHT:
					teclas[DIREITA] = true;
					break;

				case ALLEGRO_KEY_UP:
					teclas[CIMA] = true;
					break;

				case ALLEGRO_KEY_DOWN:
					teclas[BAIXO] = true;
					break;

				case ALLEGRO_KEY_BACKSPACE:
					diminuir_vida(personagem, 10);
					break;

				case ALLEGRO_KEY_ENTER:
					if (verificar_colisao(&finalizador->dimensao, &finalizador->posicao, &personagem->dimensao, &personagem->posicao)){
						nivel++;

						if (nivel < 3) {
							proximo_level = true;
						} else {
							vitoria = true;
						}
					}
					break;

				case ALLEGRO_KEY_Q:
					if (mascara->vida > 0 && toggleMascara == false) {
						mascara->usando = true;
					}else{
						mascara->usando = false;
					}
					break;

				case ALLEGRO_KEY_R:
					if (personagem->morto == true && vitoria == false) {
						personagem->morto = false;
						personagem->posicao.x = posicao_inicial.x;
						personagem->posicao.y = posicao_inicial.y;
						personagem->vida = 100;
						mascara->usando = false;
						toggleMascara = false;
					}
				break;
				}
			}

			// Verifica se a tecla foi solta
			if (evento.type == ALLEGRO_EVENT_KEY_UP) {
				switch (evento.keyboard.keycode) {
				case ALLEGRO_KEY_LEFT:
					teclas[ESQUERDA] = false;
					break;
				case ALLEGRO_KEY_RIGHT:
					teclas[DIREITA] = false;
					break;
				case ALLEGRO_KEY_UP:
					teclas[CIMA] = false;
					break;
				case ALLEGRO_KEY_DOWN:
					teclas[BAIXO] = false;
					break;
				case ALLEGRO_KEY_Q:
					if (toggleMascara == false) {
						toggleMascara = true;
					}else {
						toggleMascara = false;
					}
					break;
				}
			}

			// Fecha a aba
			if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
				rodando = false;
			}
		}
	
		if (mascara->usando && frames % 30 == 0){
			mascara->vida = usando_mascara(*mascara);
		}

		if (mascara->vida <= 0) mascara->usando = false;

		AreaTransmicao* area = colidiu_area(mapa, personagem);
		if (area != NULL && frames % 30 == 0 && mascara->usando == false) {
			diminuir_vida(personagem, 5);
		}

		Tile* tile_colidido = colidiu_mapa(mapa, personagem);
		if (tile_colidido != NULL) {

			// verifica quando colidi na vertical
			if (velocidadePersonagem.y != 0) {
				// colisao de cima para baixo
				if (velocidadePersonagem.y > 0){
					personagem->posicao.y = tile_colidido->posicao.y - personagem->dimensao.vetor.y;

					// reinicia os valores caso haja colisao
					velocidadeGravidade = 0;
					velocidadePersonagem.y = 0;
					alturaPulo = 0;
				}
				else {
					// colisao de baixo para cima
					personagem->posicao.y = (tile_colidido->posicao.y + tile_colidido->dimensao.vetor.y);
					velocidadePersonagem.y *= -REBOTE_Y;
				}
			}

			if (pegouMascara == false && tile_colidido->tipo == "mascara") {
				mascara->vida += 50;
				pegouMascara = true;
				remover_tile(mapa, tile_colidido);
			}
			

			float cantoEsqPlataforma = tile_colidido->posicao.x;
			float cantoDirPlataforma = tile_colidido->posicao.x + tile_colidido->dimensao.vetor.x;
			float cantoEsqPersonagem = personagem->posicao.x;
			float cantoDirPersonagem = personagem->posicao.x + personagem->dimensao.vetor.x;
			float eixoPersonagem = personagem->dimensao.vetor.x/2;
			
			// verifica se colidiu com o canto esquerdo com objeto em direcao a direita
			// ha um rebote com perda de forca
			if (velocidadePersonagem.x > 0 && cantoEsqPlataforma <= cantoDirPersonagem && cantoEsqPlataforma >= cantoDirPersonagem - (eixoPersonagem * 0.5)) {
				//printf("olha o rebote para esquerda");
				velocidadePersonagem.x = 0;
			}
			
			/*
			* falta ajustes
			// verifica se colidiu com o canto direito com objeto em direcao a esquerda
			// ha um rebote com perda de forca
			if (velocidadePersonagem.x < 0 && cantoDirPlataforma >= cantoEsqPersonagem && cantoDirPlataforma <= cantoEsqPersonagem + (eixoPersonagem * 0.5)) {
				printf("pqp");
				velocidadePersonagem.x = 0;
			}
			*/

			// Inicio pulo quando colidir
			if (teclas[CIMA] && alturaPulo < ALTURA_MAX_PULO) {
				velocidadePersonagem.y -= DELTA_PULO;
				alturaPulo += DELTA_PULO;
			}

			// Movimentacao no eixo x
			//enquando esta colidindo
			if ((teclas[DIREITA] || teclas[ESQUERDA])) {

				//caso esteja no intervalo entre a maior e menor velocidade
				if ((velocidadePersonagem.x < VELOCIDADE_MAX_X) && (velocidadePersonagem.x > -VELOCIDADE_MAX_X)) {
					velocidadePersonagem.x += 0.6 * teclas[DIREITA];
					velocidadePersonagem.x -= 0.6 * teclas[ESQUERDA];
				}
			} else {
				velocidadePersonagem.x *= 0.5;
			}
		}
		
		//quando nao ha colisao
		if (tile_colidido == NULL) {

			/*
			Mecanica de Pulo
			se a tecla cima for acionada
			e 
			a altura do pulo for menor que a altura maxima permitida
			e 
			nao estiver colidindo com nada
			*/
			if (teclas[CIMA] && alturaPulo < ALTURA_MAX_PULO) {
				velocidadePersonagem.y -= DELTA_PULO;
				alturaPulo += DELTA_PULO;
			}
			else {
				//se a altura max do pulo for alcancada e necessario esperar ate colidir
				teclas[CIMA] = false;
			}

			//caso seja menor que o limite de velocidade 
			if (velocidadePersonagem.y <= VELOCIDADE_MAX_Y) {
				velocidadePersonagem.y -= velocidadeGravidade - (2.2 * teclas[BAIXO]);
				velocidadeGravidade -= GRAVIDADE;
			}
			//senao continua na mesma velocidade/velocidade maxima de y

			/* 
			//Movimentacao no eixo x
			enquando esta colidindo
			*/
			if ((teclas[DIREITA] || teclas[ESQUERDA])) {

				//caso esteja no intervalo entre a maior e menor velocidade
				if ((velocidadePersonagem.x < VELOCIDADE_MAX_X) && (velocidadePersonagem.x > -VELOCIDADE_MAX_X)) {
					velocidadePersonagem.x += 0.6 * teclas[DIREITA];
					velocidadePersonagem.x -= 0.6 * teclas[ESQUERDA];
				}
			}
			else {
				velocidadePersonagem.x *= 0.5;
			}
		}
		/*
		movimentacao personagem
		baseada no vetor do personagem
		*/
		personagem->posicao.y += velocidadePersonagem.y;
		personagem->posicao.x += velocidadePersonagem.x;

		if (personagem->posicao.y > 580) {
			diminuir_vida(personagem, 100);
		}

		if (personagem->morto)
			mascara->usando = false;

		// Verica se � necessario limpar a tela
		if (desenhar && al_is_event_queue_empty(fila_eventos)) {
			al_clear_to_color(COR_FUNDO);
			
			if (personagem->morto) {
				desenhar_sprite(gameover_sprite, &aux);
			} else {
				if (vitoria) {
					desenhar_sprite(vitoria_sprite, &aux);
				} else {
					desenhar_mapa(mapa);
					desenhar_personagem(personagem);
					desenhar_sprite(vida_sprite, &aux);
					al_draw_textf(fonte, al_map_rgb(0, 0, 0), 18, 0, 0, "%d", (personagem->vida - 100) * -1);
					desenhar_sprite(mascara_sprite, &aux2);
					al_draw_textf(fonte, al_map_rgb(0, 0, 0), 70, 0, 0, "%d %s", mascara->vida, (mascara->usando)?"usando":"");
					al_draw_textf(fonte, al_map_rgb(0, 0, 0), posicao_texto.x, posicao_texto.y, 0, "%s", texto);
				}
			}

			al_flip_display();
			desenhar = false;
		}

		if (proximo_level && !vitoria) {
			liberar_mapa(mapa);
			mapa = carregar_mapa(niveis[nivel]);
			proximo_level = false;
			personagem->posicao.x = posicao_inicial.x;
			personagem->posicao.y = posicao_inicial.y;
			if (nivel == 1)
				texto = "evite aglomeracao";

			if (nivel == 2)
				texto = "fique em casa";
		}
	}


	// Limpa tudo
	liberar_mapa(mapa);
	destruir_tile_sheet();
	al_destroy_bitmap(personagem->sprite->imagem);
	al_destroy_audio_stream(musicaFundo.som);
	al_destroy_event_queue(fila_eventos);
	al_destroy_font(fonte);
	al_destroy_display(janela);
	return 0;
}