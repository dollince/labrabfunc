#define _USE_MATH_DEFINES
#include <stdio.h>
#include <math.h>
#include <locale.h>
#include <stdlib.h>


typedef struct {
	float (*firstterm)(float);
	float (*nextterm)(float, float, int);
} Series;

void prev(float* x) {
	*x = *x - floorf(*x / (2 * (float)M_PI)) * 2 * (float)M_PI;
}
//sin
float sin_first(float x) {
	return x;
}
float sin_next(float prev, float x, int n) {
	return -prev * x * x / (((2.0f * n) * (2.0f * n + 1.0f)));
}
//cos
float cos_first(float x) {
	return 1.0f;
}
float cos_next(float prev, float x, int n) {
	return -prev * x * x / (((2.0f * n - 1.0f) * (2.0f * n)));
}
//exp
float exp_first(float x) {
	return 1.0f;
}
float exp_next(float prev, float x, int n) {
	return prev * x / n;
}
//ln
float ln_first(float x) {
	float t = (x - 1.0f) / (x + 1.0f);
	return 2.0f * t;
}
float ln_next(float prev, float x, int n) {
	float t = (x - 1.0f) / (x + 1.0f);
	return prev * t * t * (2.0f * n - 1.0f) / (2.0f * n + 1.0f);
}
//прямое
float sum_direct(Series s, float x, int max_n) {
	float sum = 0.0f;
	float term = s.firstterm(x);
	for (int n = 1; n <= max_n; n++) {
		sum += term;
		term = s.nextterm(term, x, n);
	}
	return sum;
}
//обратное
float sum_back(Series s, float x, int max_n) {
	float* terms = (float*)malloc(max_n * sizeof(float));
	if (terms == NULL) {
		printf("Ошибка выделения памяти в обратном суммировании.\n");
		return 0.0f;
	}
	int count = 0;
	float term = s.firstterm(x);
	for (int n = 1; n <= max_n; n++) {
		terms[count++] = term;
		term = s.nextterm(term, x, n);
	}
	float sum = 0.0f;
	for (int i = count - 1; i >= 0; i--) {
		sum += terms[i];
	}
	free(terms);
	return sum;
}
//алгоритм кэхана
float sum_kahan(Series s, float x, int max_n) {
	float sum = s.firstterm(x);
	float tmp = sum;
	float comp = 0.0f; 

	for (int n = 2; n <= max_n; n++) {
		tmp = s.nextterm(tmp, x, n - 1);

		float c = sum + tmp;
		float t1 = c - tmp;
		t1 = sum - t1;
		float t2 = c - sum;
		t2 = tmp - t2; 
		float err = t1 + t2;

		comp += err;
		sum += tmp;
	}
	sum += comp;
	return sum;
}
void printfunc(Series s, const char* func_name, float x, int max_n, float (*math_func)(float)) {
	printf("%s\n", func_name);
	printf("%f, %d\n\n", x, max_n);
	float direct_result = sum_direct(s, x, max_n);
	float back_result = sum_back(s, x, max_n);
	float kahan_result = sum_kahan(s, x, max_n);
	printf("Прямое суммирование  %f\n", direct_result);
	printf("Обратное суммирование %f\n", back_result);
	printf("Метод Кэхана %f\n", kahan_result);
	printf("Math.h библиотека:  %f\n\n", math_func(x));
	printf("Погрешность\n");
	printf("Прямое:  %e\n", fabsf(direct_result - math_func(x)));
	printf("Обратное: %e\n", fabsf(back_result - math_func(x)));
	printf("Кэхан: %e\n", fabsf(kahan_result - math_func(x)));

}

int main() {
	int choice;
	float x;
	int max_n;
	int choice2 = 1;
	setlocale(LC_ALL, "rus");
	
	do {
		printf("\nВыберите функцию\n");
		printf("0. Выход\n");
		printf("1. sin(x)\n");
		printf("2. cos(x)\n");
		printf("3. exp(x)\n");
		printf("4. ln(x)\n");
		printf("Выбор:\n");
		scanf_s("%d", &choice);

		if (choice == 0) {
			break;
		}
		if (choice < 1 || choice>4) {
			printf("\nНеверный ввод.\n");
			break;
		}
		printf("Введите x:\n");
		scanf_s("%f", &x);

		if (choice == 4 && x <= 0) {
			printf("\nx должен быть больше 0.\n");
			break;
		}
		printf("Введите максимальное число итераций n:\n");
		scanf_s("%d", &max_n);

		if (max_n <= 0) {
			printf("\nn должно быть положительным числом.\n");
			break;
		}
		Series s;
		const char* func_name;
		float (*math_func)(float);

		switch (choice) {
		case 1:
			prev(&x);
			s.firstterm = sin_first;
			s.nextterm = sin_next;
			func_name = "sin(x)";
			math_func = sinf;
			printfunc(s, func_name, x, max_n, math_func);
			break;
		case 2:
			prev(&x);
			s.firstterm = cos_first;
			s.nextterm = cos_next;
			func_name = "cos(x)";
			math_func = cosf;
			printfunc(s, func_name, x, max_n, math_func);
			break;
		case 3:
			s.firstterm = exp_first;
			s.nextterm = exp_next;
			func_name = "exp(x)";
			math_func = expf;
			printfunc(s, func_name, x, max_n, math_func);
			break;
		case 4:
			s.firstterm = ln_first;
			s.nextterm = ln_next;
			func_name = "ln(x)";
			math_func = logf;
			printfunc(s, func_name, x, max_n, math_func);
			break;
		}

		printf("Хочешь продолжить?\n");
		printf("1 - Да\n0 - Нет\n");
		scanf_s("%d", &choice2);
	} while(choice2 == 1);
		
	return 0;
}