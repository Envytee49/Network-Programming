-- Tạo bảng users
CREATE TABLE IF NOT EXISTS public.users
(
    user_id SERIAL PRIMARY KEY,
    username VARCHAR(30) NOT NULL,
    password VARCHAR(256) NOT NULL,
    email VARCHAR(255) NOT NULL UNIQUE,
    created_at TIMESTAMP WITHOUT TIME ZONE DEFAULT CURRENT_TIMESTAMP
)
TABLESPACE pg_default;

-- Tạo bảng sessions
CREATE TABLE IF NOT EXISTS public.sessions
(
    id SERIAL PRIMARY KEY,
    user_id INTEGER,
    token VARCHAR(255) NOT NULL UNIQUE,
    created_at TIMESTAMP WITHOUT TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT fk_user_sessions FOREIGN KEY (user_id)
        REFERENCES public.users (user_id) MATCH SIMPLE
        ON UPDATE NO ACTION
        ON DELETE CASCADE
)
TABLESPACE pg_default;

-- Tạo hàm delete_old_token_and_insert_new để xóa token cũ khi insert token mới
CREATE OR REPLACE FUNCTION public.delete_old_token_and_insert_new()
    RETURNS trigger
    LANGUAGE 'plpgsql'
    COST 100
    VOLATILE NOT LEAKPROOF
AS $BODY$
BEGIN
    -- Kiểm tra xem token của user_id đã tồn tại trong bảng sessions chưa
    IF EXISTS (SELECT 1 FROM sessions WHERE user_id = NEW.user_id) THEN
        -- Xóa token cũ của user_id đó
        DELETE FROM sessions WHERE user_id = NEW.user_id;
    END IF;

    -- Trả về NEW để tiếp tục thực thi thao tác insert token mới
    RETURN NEW;
END;
$BODY$;

-- Tạo trigger update_token_trigger để gọi hàm khi insert vào bảng sessions
CREATE OR REPLACE TRIGGER update_token_trigger
    BEFORE INSERT
    ON public.sessions
    FOR EACH ROW
    EXECUTE FUNCTION public.delete_old_token_and_insert_new();



select * from users

select * from sessions