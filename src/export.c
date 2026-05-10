#include "export.h"
#include <stdio.h>

#ifdef HAVE_XLSXWRITER
#include <xlsxwriter.h>

int export_to_excel(const char *path, const List *list) {
    lxw_workbook  *wb = workbook_new(path);
    lxw_worksheet *ws = workbook_add_worksheet(wb, NULL);
    int ok = (wb != NULL && ws != NULL);
    if (ok) {
        worksheet_write_string(ws, 0, 0, "Группа",       NULL);
        worksheet_write_string(ws, 0, 1, "Код",          NULL);
        worksheet_write_string(ws, 0, 2, "Наименование", NULL);
        worksheet_write_string(ws, 0, 3, "Модель",       NULL);
        worksheet_write_string(ws, 0, 4, "Цена",         NULL);
        worksheet_write_string(ws, 0, 5, "Количество",   NULL);

        int row = 1;
        Node *cur = list->head;
        while (cur != NULL) {
            worksheet_write_string(ws, row, 0, cur->data.group, NULL);
            worksheet_write_number(ws, row, 1, cur->data.code,  NULL);
            worksheet_write_string(ws, row, 2, cur->data.name,  NULL);
            worksheet_write_string(ws, row, 3, cur->data.model, NULL);
            worksheet_write_number(ws, row, 4, cur->data.price, NULL);
            worksheet_write_number(ws, row, 5, cur->data.quantity, NULL);
            row++;
            cur = cur->next;
        }
        workbook_close(wb);
    }
    return ok;
}

#else

int export_to_excel(const char *path, const List *list) {
    FILE *f = fopen(path, "w");
    int ok = 0;
    if (f != NULL) {
        ok = 1;
        fprintf(f, "Группа,Код,Наименование,Модель,Цена,Количество\n");
        Node *cur = list->head;
        while (cur != NULL) {
            fprintf(f, "%s,%d,%s,%s,%.2f,%d\n",
                    cur->data.group,
                    cur->data.code,
                    cur->data.name,
                    cur->data.model,
                    cur->data.price,
                    cur->data.quantity);
            cur = cur->next;
        }
        fclose(f);
    }
    return ok;
}

#endif
