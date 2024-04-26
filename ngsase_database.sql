/*建立ngsase 資料庫*/
CREATE DATABASE IF NOT EXISTS ngsase;
/*使用database*/
USE ㄑㄑ
WHERE SCHEMA_NAME = 'ngsase';

SELECT *
FROM information_schema.TABLES
WHERE TABLE_SCHEMA = 'ngsase'
  AND TABLE_NAME = 'users';

/*建立users 表格用於登入與註冊使用*/
CREATE TABLE IF NOT EXISTS users(
  email VARCHAR(255) NOT NULL PRIMARY KEY,
  password VARCHAR(50) NOT NULL
);
/*建立pipelines表格用於pipelines讀取使用
  記得pipelines pipeline_name, step ,email 要加上index視為一體(複合索引)
  切記複合索引會優先考慮第一個順位的欄位，也就是會以第一個欄位搭配後續的欄位
  像是 A B C
  A+ (B+C)
  A +(B)*/
CREATE TABLE IF NOT EXISTS pipelines(
 pipeline_name VARCHAR(255) NOT NULL,
 tool VARCHAR(255) NOT NULL,
 setting JSON NOT NULL,
 step INT CHECK (step>0) NOT NULL,
 email VARCHAR(255) NOT NULL,
 PRIMARY KEY(pipeline_name,email,step),
 FOREIGN KEY(email) REFERENCES users(email),
 INDEX idx_pipeline_step_email (pipeline_name, step, email) 
);


/*建立project table*/
CREATE TABLE IF NOT EXISTS projects(
  uuid VARCHAR(36) NOT NULL,
  project_name VARCHAR(255) NOT NULL,
  pipeline_name VARCHAR(255) NOT NULL,
  paired_end BOOLEAN NOT NULL,
  input_directory TEXT NOT NULL,
  output_directory TEXT NOT NULL,
  project_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  finished BOOLEAN NOT NULL,
  email VARCHAR(255) NOT NULL,
  PRIMARY KEY(project_name,pipeline_name,email),
  FOREIGN KEY(pipeline_name,email) REFERENCES pipelines(pipeline_name,email)
);

/*建立index table*/
CREATE TABLE IF NOT EXISTS genomeIndexs(
  index_name VARCHAR(255) NOT NULL,
  index_directory TEXT NOT NULL,
  tool VARCHAR(255) NOT NULL,
  email VARCHAR(255) NOT NULL,
  PRIMARY KEY(index_name,email,tool),
  FOREIGN KEY(email) REFERENCES users(email)
);

/*建立gtf table*/
CREATE TABLE IF NOT EXISTS gtfs(
  gtf_name VARCHAR(255) NOT NULL,
  gtf_directory TEXT NOT NULL,
  email VARCHAR(255) NOT NULL,
  PRIMARY KEY(gtf_name,email),
  FOREIGN KEY(email) REFERENCES users(email)
);


